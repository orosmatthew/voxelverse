#include "text_pipeline.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "logger.hpp"
#include "mve/common.hpp"
#include "text_buffer.hpp"

TextPipeline::TextPipeline(mve::Renderer& renderer)
    : m_renderer(&renderer)
    , m_vert_shader("../res/bin/shader/text.vert.spv")
    , m_frag_shader("../res/bin/shader/text.frag.spv")
    , m_pipeline(renderer, m_vert_shader, m_frag_shader, c_vertex_layout)
    , m_global_ubo(renderer, m_vert_shader.descriptor_set(0).binding(0))
    , m_global_descriptor_set(renderer, m_pipeline, m_vert_shader.descriptor_set(0))
    , m_model_location(m_vert_shader.descriptor_set(1).binding(0).member("model").location())
    , m_text_color_location(m_vert_shader.descriptor_set(1).binding(0).member("text_color").location())
    , m_texture_binding(m_frag_shader.descriptor_set(1).binding(1))
    , m_glyph_ubo_binding(m_vert_shader.descriptor_set(1).binding(0))
{
    m_global_descriptor_set.write_binding(m_vert_shader.descriptor_set(0).binding(0), m_global_ubo);

    mve::VertexData vertex_data(c_vertex_layout);
    vertex_data.push_back({ 0.0f, -1.0f, 0.0f });
    vertex_data.push_back({ 0.0f, 0.0f });
    vertex_data.push_back({ 1.0f, -1.0f, 0.0f });
    vertex_data.push_back({ 1.0f, 0.0f });
    vertex_data.push_back({ 1.0f, 0.0f, 0.0f });
    vertex_data.push_back({ 1.0f, 1.0f });
    vertex_data.push_back({ 0.0f, 0.0f, 0.0f });
    vertex_data.push_back({ 0.0f, 1.0f });
    m_vertex_buffer = renderer.create_vertex_buffer(vertex_data);
    m_index_buffer = renderer.create_index_buffer({ 0, 3, 2, 0, 2, 1 });

    m_global_ubo.update(m_vert_shader.descriptor_set(0).binding(0).member("view").location(), mve::Matrix4::identity());

    FT_Error result;
    FT_Library font_lib;
    result = FT_Init_FreeType(&font_lib);
    MVE_ASSERT(result == 0, "[Text Pipeline] Failed to init FreeType")

    FT_Face font_face;
    result = FT_New_Face(font_lib, "../res/karma_suture.otf", 0, &font_face);
    MVE_ASSERT(result == 0, "[Text Pipeline] Failed to load font")

    FT_Set_Pixel_Sizes(font_face, 0, 36);

    for (unsigned char c = 0; c < 128; c++) {
        result = FT_Load_Char(font_face, c, FT_LOAD_RENDER);
        if (result != 0) {
            LOG->warn("[Text Pipeline] Failed to load glyph: {}", std::to_string(c));
            continue;
        }
        mve::Texture texture;
        if (font_face->glyph->bitmap.width != 0 && font_face->glyph->bitmap.rows != 0) {
            texture = renderer.create_texture(
                mve::TextureFormat::r,
                font_face->glyph->bitmap.width,
                font_face->glyph->bitmap.rows,
                reinterpret_cast<const std::byte*>(font_face->glyph->bitmap.buffer));
        }
        else {
            std::byte val {};
            texture = renderer.create_texture(mve::TextureFormat::r, 1, 1, &val);
        }
        FontChar font_char
            = { .texture = std::move(texture),
                .size
                = { static_cast<int>(font_face->glyph->bitmap.width), static_cast<int>(font_face->glyph->bitmap.rows) },
                .bearing = { font_face->glyph->bitmap_left, font_face->glyph->bitmap_top },
                .advance = static_cast<uint32_t>(font_face->glyph->advance.x) };
        m_font_chars.insert({ c, std::move(font_char) });
    }
    FT_Done_Face(font_face);
    FT_Done_FreeType(font_lib);
}

void TextPipeline::resize()
{
    mve::Matrix4 proj = mve::ortho(
        -static_cast<float>(m_renderer->extent().x) / 2.0f,
        static_cast<float>(m_renderer->extent().x) / 2.0f,
        -static_cast<float>(m_renderer->extent().y) / 2.0f,
        static_cast<float>(m_renderer->extent().y) / 2.0f,
        -1000.0f,
        1000.0f);
    m_global_ubo.update(m_vert_shader.descriptor_set(0).binding(0).member("proj").location(), proj);
}
TextBuffer TextPipeline::create_text_buffer()
{
    auto it
        = std::find_if(m_text_buffers.begin(), m_text_buffers.end(), [](const std::optional<TextBufferImpl>& buffer) {
              return !buffer.has_value();
          });
    TextBuffer buffer;
    buffer.m_valid = true;
    buffer.m_pipeline = this;
    if (it != m_text_buffers.end()) {
        buffer.m_handle = it - m_text_buffers.begin();
        *it = TextBufferImpl {};
    }
    else {
        buffer.m_handle = m_text_buffers.size();
        m_text_buffers.push_back(TextBufferImpl {});
    }
    return buffer;
}
void TextPipeline::destroy(TextBuffer& buffer)
{
    if (!buffer.m_valid) {
        return;
    }
    m_text_buffers[buffer.m_handle].reset();
    buffer.m_valid = false;
}

void TextPipeline::update_text_buffer(const TextBuffer& buffer, std::string_view text, mve::Vector2 pos, float scale)
{
    MVE_VAL_ASSERT(buffer.m_valid, "[Text Pipeline] Text buffer invalid")
    // TODO: Find a better way to do this
    for (RenderGlyph& glyph : *m_text_buffers[buffer.m_handle]) {
        glyph.is_valid = false;
    }
    for (auto c = text.begin(); c != text.end(); c++) {
        if (!m_font_chars.contains(*c)) {
            LOG->warn("[Text Pipeline] Invalid char: {}", static_cast<uint32_t>(*c));
            continue;
        }
        const FontChar& font_char = m_font_chars.at(*c);
        float x_pos = pos.x + font_char.bearing.x * scale;
        float y_pos = pos.y - (font_char.size.y - font_char.bearing.y) * scale;
        float w = font_char.size.x * scale;
        float h = font_char.size.y * scale;

        mve::Matrix4 model = mve::Matrix4::identity();
        model = model.scale({ w, h, 1.0f });
        model = model.translate({ x_pos, -y_pos, 0.0f });

        MVE_VAL_ASSERT(m_text_buffers.at(buffer.m_handle).has_value(), "[Text Pipeline] Text buffer invalid")
        TextBufferImpl& buffer_impl = *m_text_buffers[buffer.m_handle];

        auto it = std::find_if(buffer_impl.begin(), buffer_impl.end(), [](const RenderGlyph& glyph) {
            return !glyph.is_valid;
        });
        if (it != buffer_impl.end()) {
            it->ubo.update(m_model_location, model);
            it->ubo.update(m_text_color_location, mve::Vector3(0.0f, 0.0f, 0.0f)); // TODO: Ability to change color
            it->descriptor_set.write_binding(m_texture_binding, font_char.texture);
            it->character = *c;
            it->translation = pos;
            it->scale = scale;
            it->is_valid = true;
        }
        else {
            mve::UniformBuffer glyph_ubo = m_renderer->create_uniform_buffer(m_glyph_ubo_binding);
            mve::DescriptorSet glyph_descriptor_set = m_pipeline.create_descriptor_set(m_vert_shader.descriptor_set(1));
            glyph_descriptor_set.write_binding(m_glyph_ubo_binding, glyph_ubo);
            RenderGlyph render_glyph { .is_valid = true,
                                       .ubo = std::move(glyph_ubo),
                                       .descriptor_set = std::move(glyph_descriptor_set),
                                       .character = *c,
                                       .translation = pos,
                                       .scale = scale };
            render_glyph.ubo.update(m_model_location, model);
            render_glyph.ubo.update(m_text_color_location, mve::Vector3(0.0f, 0.0f, 0.0f)); // TODO
            render_glyph.descriptor_set.write_binding(m_texture_binding, font_char.texture);
            buffer_impl.push_back(std::move(render_glyph));
        }
        pos.x += mve::floor(font_char.advance / 64.0f) * scale;
    }
}

void TextPipeline::draw(const TextBuffer& buffer) const
{
    MVE_VAL_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to draw invalid text buffer")
    m_renderer->bind_graphics_pipeline(m_pipeline); // TODO: Determine if this is the best way to do this
    m_renderer->bind_vertex_buffer(m_vertex_buffer);
    MVE_VAL_ASSERT(
        m_text_buffers.at(buffer.m_handle).has_value(), "[Text Pipeline] Attempt to draw invalid text buffer")
    const TextBufferImpl& buffer_impl = *m_text_buffers[buffer.m_handle];
    for (const RenderGlyph& glyph : buffer_impl) {
        if (glyph.is_valid) {
            m_renderer->bind_descriptor_sets(m_global_descriptor_set, glyph.descriptor_set);
            m_renderer->draw_index_buffer(m_index_buffer);
        }
    }
}

void TextPipeline::set_text_buffer_translation(const TextBuffer& buffer, mve::Vector2 pos)
{
    MVE_VAL_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to set translation on invalid text buffer")
    TextBufferImpl& buffer_impl = *m_text_buffers[buffer.m_handle];

    for (RenderGlyph& glyph : buffer_impl) {
        MVE_VAL_ASSERT(m_font_chars.contains(glyph.character), "[Text Pipeline] Attempt to update invalid character")
        const FontChar& font_char = m_font_chars.at(glyph.character);
        float x_pos = pos.x + font_char.bearing.x * glyph.scale;
        float y_pos = pos.y - (font_char.size.y - font_char.bearing.y) * glyph.scale;
        float w = font_char.size.x * glyph.scale;
        float h = font_char.size.y * glyph.scale;
        mve::Matrix4 model = mve::Matrix4::identity();
        model = model.scale({ w, h, 1.0f });
        model = model.translate({ x_pos, -y_pos, 0.0f });
        glyph.ubo.update(m_model_location, model);
        glyph.translation = pos;
        pos.x += mve::floor(font_char.advance / 64.0f) * glyph.scale;
    }
}
