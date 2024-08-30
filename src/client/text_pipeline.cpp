#include "text_pipeline.hpp"

#include "common.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../common/assert.hpp"
#include "../common/logger.hpp"
#include "text_buffer.hpp"

TextPipeline::TextPipeline(mve::Renderer& renderer, const int point_size)
    : m_renderer(&renderer)
    , m_vert_shader(res_path("bin/shader/text.vert.spv"))
    , m_frag_shader(res_path("bin/shader/text.frag.spv"))
    , m_pipeline(renderer, m_vert_shader, m_frag_shader, c_vertex_layout)
    , m_global_ubo(renderer, m_vert_shader.descriptor_set(0).binding(0))
    , m_global_descriptor_set(renderer, m_pipeline, m_vert_shader.descriptor_set(0))
    , m_model_location(m_vert_shader.descriptor_set(1).binding(0).member("model").location())
    , m_text_color_location(m_vert_shader.descriptor_set(1).binding(0).member("text_color").location())
    , m_texture_binding(m_frag_shader.descriptor_set(1).binding(1))
    , m_glyph_ubo_binding(m_vert_shader.descriptor_set(1).binding(0))
    , c_point_size(point_size)
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

    m_global_ubo.update(
        m_vert_shader.descriptor_set(0).binding(0).member("view").location(),
        nnm::Transform3f().translate({ 0.0f, 0.0f, -1.0f }).matrix);

    mve::VertexData cursor_data(c_vertex_layout);
    cursor_data.push_back({ -0.05f, -1.0f, 0.0f });
    cursor_data.push_back({ 0.0f, 0.0f });
    cursor_data.push_back({ 0.05f, -1.0f, 0.0f });
    cursor_data.push_back({ 1.0f, 0.0f });
    cursor_data.push_back({ 0.05f, 0.0f, 0.0f });
    cursor_data.push_back({ 1.0f, 1.0f });
    cursor_data.push_back({ -0.05f, 0.0f, 0.0f });
    cursor_data.push_back({ 0.0f, 1.0f });
    m_cursor_vertex_buffer = renderer.create_vertex_buffer(cursor_data);
    m_cursor_index_buffer = renderer.create_index_buffer({ 0, 3, 2, 0, 2, 1 });

    FT_Library font_lib;
    FT_Error result = FT_Init_FreeType(&font_lib);
    VV_REL_ASSERT(result == 0, "[Text Pipeline] Failed to init FreeType")

    FT_Face font_face;
    const std::string font_path = res_path("matrix_sans_video.otf").string();
    result = FT_New_Face(font_lib, font_path.c_str(), 0, &font_face);
    VV_REL_ASSERT(result == 0, "[Text Pipeline] Failed to load font")

    FT_Set_Pixel_Sizes(font_face, 0, c_point_size);

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

    constexpr std::byte cursor_texture_data { 255 };
    m_cursor_texture = renderer.create_texture(mve::TextureFormat::r, 1, 1, &cursor_texture_data);
}

void TextPipeline::resize()
{
    const auto proj = nnm::Transform3f::from_projection_orthographic(
        0.0f,
        static_cast<float>(m_renderer->extent().x),
        0.0f,
        static_cast<float>(m_renderer->extent().y),
        -1000.0f,
        1000.0f);
    m_global_ubo.update(m_vert_shader.descriptor_set(0).binding(0).member("proj").location(), proj.matrix);
}
TextBuffer TextPipeline::create_text_buffer()
{
    return create_text_buffer("", { 0.0f, 0.0f }, 1.0f, { 0.0f, 0.0f, 0.0f });
}
void TextPipeline::destroy(TextBuffer& buffer)
{
    if (!buffer.m_valid) {
        return;
    }
    m_text_buffers[buffer.m_handle].reset();
    buffer.m_valid = false;
}

void TextPipeline::draw(const TextBuffer& buffer) const
{
    VV_DEB_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to draw invalid text buffer")
    m_renderer->bind_graphics_pipeline(m_pipeline); // TODO: Determine if this is the best way to do this
    m_renderer->bind_vertex_buffer(m_vertex_buffer);
    VV_DEB_ASSERT(m_text_buffers.at(buffer.m_handle).has_value(), "[Text Pipeline] Attempt to draw invalid text buffer")
    // ReSharper disable once CppUseStructuredBinding
    const TextBufferImpl& buffer_impl = *m_text_buffers[buffer.m_handle];
    // ReSharper disable once CppUseStructuredBinding
    for (const RenderGlyph& glyph : buffer_impl.render_glyphs) {
        if (glyph.is_valid) {
            m_renderer->bind_descriptor_sets(m_global_descriptor_set, glyph.descriptor_set);
            m_renderer->draw_index_buffer(m_index_buffer);
        }
    }
    if (buffer_impl.cursor.has_value()) {
        m_renderer->bind_descriptor_sets(m_global_descriptor_set, buffer_impl.cursor->descriptor_set);
        m_renderer->bind_vertex_buffer(m_cursor_vertex_buffer);
        m_renderer->draw_index_buffer(m_cursor_index_buffer);
    }
}

void TextPipeline::set_text_buffer_translation(const TextBuffer& buffer, nnm::Vector2f pos)
{
    VV_DEB_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to set translation on invalid text buffer")
    // ReSharper disable once CppUseStructuredBinding
    TextBufferImpl& buffer_impl = *m_text_buffers[buffer.m_handle];
    buffer_impl.translation = pos;
    // ReSharper disable once CppUseStructuredBinding
    for (RenderGlyph& glyph : buffer_impl.render_glyphs) {
        VV_DEB_ASSERT(m_font_chars.contains(glyph.character), "[Text Pipeline] Attempt to update invalid character")
        const auto& [texture, size, bearing, advance] = m_font_chars.at(glyph.character);
        float x_pos = pos.x + static_cast<float>(bearing.x) * glyph.scale;
        float y_pos = pos.y - static_cast<float>(bearing.y - size.y) * glyph.scale + static_cast<float>(c_point_size);
        float w = static_cast<float>(size.x) * glyph.scale;
        float h = static_cast<float>(size.y) * glyph.scale;
        nnm::Transform3f model;
        model = model.scale({ w, h, 1.0f });
        model = model.translate({ x_pos, y_pos, 0.0f });
        glyph.ubo.update(m_model_location, model.matrix);
        glyph.translation = pos;
        pos.x += nnm::floor(static_cast<float>(advance) / 64.0f) * glyph.scale;
    }
}
void TextPipeline::add_cursor(const TextBuffer& buffer, const int pos)
{
    VV_DEB_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to add cursor on invalid text buffer")
    auto& [render_glyphs, cursor, cursor_pos, translation, scale, text_length, color, text]
        = *m_text_buffers[buffer.m_handle];
    cursor_pos = pos;
    cursor = { .ubo = m_renderer->create_uniform_buffer(m_glyph_ubo_binding),
               .descriptor_set = m_pipeline.create_descriptor_set(m_vert_shader.descriptor_set(1)),
               .translation = translation };
    cursor->ubo.update(m_text_color_location, nnm::Vector3f::zero());
    cursor->descriptor_set.write_binding(m_glyph_ubo_binding, cursor->ubo);
    cursor->descriptor_set.write_binding(m_texture_binding, m_cursor_texture);
    set_cursor_pos(buffer, pos);
}
void TextPipeline::set_cursor_pos(const TextBuffer& buffer, int pos)
{
    VV_DEB_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to set cursor position on invalid text buffer")
    auto& [render_glyphs, cursor, cursor_pos, translation, scale, text_length, color, text]
        = *m_text_buffers[buffer.m_handle];
    pos = std::clamp(pos, 0, text_length);
    cursor_pos = pos;
    float x = translation.x;
    for (int i = 0; i < render_glyphs.size() && i < pos; i++) {
        x += nnm::floor(static_cast<float>(m_font_chars[render_glyphs[i].character].advance) / 64.0f) * scale;
    }
    const nnm::Transform3f model
        = nnm::Transform3f()
              .scale(nnm::Vector3f::all(scale * static_cast<float>(c_point_size)))
              .translate({ x, translation.y + static_cast<float>(c_point_size), 0.1f });
    cursor->ubo.update(m_model_location, model.matrix);
}
void TextPipeline::remove_cursor(const TextBuffer& buffer)
{
    VV_DEB_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to remove cursor on invalid text buffer")
    // ReSharper disable once CppUseStructuredBinding
    TextBufferImpl& buffer_impl = *m_text_buffers[buffer.m_handle];
    buffer_impl.cursor.reset();
}
std::optional<int> TextPipeline::cursor_pos(const TextBuffer& buffer)
{
    VV_DEB_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to get cursor position on invalid text buffer")
    if (m_text_buffers[buffer.m_handle]->cursor.has_value()) {
        return m_text_buffers[buffer.m_handle]->cursor_pos;
    }
    return {};
}

void TextPipeline::update_text_buffer(const TextBuffer& buffer, const std::string_view text)
{
    VV_DEB_ASSERT(buffer.m_valid, "[Text Pipeline] Text buffer invalid")
    // TODO: Find a better way to do this
    // ReSharper disable once CppUseStructuredBinding
    for (RenderGlyph& glyph : m_text_buffers[buffer.m_handle]->render_glyphs) {
        glyph.is_valid = false;
    }
    VV_DEB_ASSERT(m_text_buffers.at(buffer.m_handle).has_value(), "[Text Pipeline] Text buffer invalid")
    // ReSharper disable once CppUseStructuredBinding
    TextBufferImpl& buffer_impl = *m_text_buffers[buffer.m_handle];
    if (buffer_impl.text == text) {
        return;
    }
    nnm::Vector2 pos = buffer_impl.translation;
    buffer_impl.text_length = static_cast<int>(text.length());
    const float scale = buffer_impl.scale;
    for (char c : text) {
        if (!m_font_chars.contains(c)) {
            LOG->warn("[Text Pipeline] Invalid char: {}", static_cast<uint32_t>(c));
            continue;
        }
        const auto& [texture, size, bearing, advance] = m_font_chars.at(c);
        float x_pos = pos.x + static_cast<float>(bearing.x) * scale;
        float y_pos = pos.y - static_cast<float>(bearing.y - size.y) * scale + static_cast<float>(c_point_size);
        float w = static_cast<float>(size.x) * scale;
        float h = static_cast<float>(size.y) * scale;

        nnm::Transform3f model;
        model = model.scale({ w, h, 1.0f });
        model = model.translate({ x_pos, y_pos, 0.0f });

        if (auto it
            = std::ranges::find_if(buffer_impl.render_glyphs, [](const RenderGlyph& glyph) { return !glyph.is_valid; });
            it != buffer_impl.render_glyphs.end()) {
            it->ubo.update(m_model_location, model.matrix);
            it->ubo.update(m_text_color_location, buffer_impl.color);
            it->descriptor_set.write_binding(m_texture_binding, texture);
            it->character = c;
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
                                       .character = c,
                                       .translation = pos,
                                       .scale = scale };
            render_glyph.ubo.update(m_model_location, model.matrix);
            render_glyph.ubo.update(m_text_color_location, buffer_impl.color);
            render_glyph.descriptor_set.write_binding(m_texture_binding, texture);
            buffer_impl.render_glyphs.push_back(std::move(render_glyph));
        }
        pos.x += nnm::floor(static_cast<float>(advance) / 64.0f) * scale;
    }
    if (buffer_impl.cursor.has_value()) {
        set_cursor_pos(buffer, buffer_impl.cursor_pos);
    }
}

void TextPipeline::cursor_left(const TextBuffer& buffer)
{
    if (const std::optional<int> pos = cursor_pos(buffer); pos.has_value()) {
        set_cursor_pos(buffer, *pos - 1);
    }
}

void TextPipeline::cursor_right(const TextBuffer& buffer)
{
    if (const std::optional<int> pos = cursor_pos(buffer); pos.has_value()) {
        set_cursor_pos(buffer, *pos + 1);
    }
}

void TextPipeline::set_text_buffer_scale(const TextBuffer& buffer, const float scale)
{
    VV_DEB_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to set translation on invalid text buffer")
    // ReSharper disable once CppUseStructuredBinding
    TextBufferImpl& buffer_impl = *m_text_buffers[buffer.m_handle];
    buffer_impl.scale = scale;
    nnm::Vector2 pos = buffer_impl.translation;
    // ReSharper disable once CppUseStructuredBinding
    for (RenderGlyph& glyph : buffer_impl.render_glyphs) {
        VV_DEB_ASSERT(m_font_chars.contains(glyph.character), "[Text Pipeline] Attempt to update invalid character")
        const auto& [texture, size, bearing, advance] = m_font_chars.at(glyph.character);
        float x_pos = pos.x + static_cast<float>(bearing.x) * glyph.scale;
        float y_pos = pos.y - static_cast<float>(bearing.y - size.y) * scale + static_cast<float>(c_point_size);
        float w = static_cast<float>(size.x) * glyph.scale;
        float h = static_cast<float>(size.y) * glyph.scale;
        nnm::Transform3f model;
        model = model.scale({ w, h, 1.0f });
        model = model.translate({ x_pos, y_pos, 0.0f });
        glyph.ubo.update(m_model_location, model.matrix);
        glyph.translation = pos;
        pos.x += nnm::floor(static_cast<float>(advance) / 64.0f) * glyph.scale;
    }
}

TextBuffer TextPipeline::create_text_buffer(
    const std::string_view text, const nnm::Vector2f pos, const float scale, const nnm::Vector3f color)
{
    const auto it = std::ranges::find_if(m_text_buffers, [](const std::optional<TextBufferImpl>& buffer) {
        return !buffer.has_value();
    });
    TextBuffer buffer;
    buffer.m_valid = true;
    buffer.m_pipeline = this;
    TextBufferImpl buffer_impl
        = { .render_glyphs = {},
            .cursor = {},
            .translation = { 0.0f, 0.0f },
            .scale = 1.0f,
            .text_length = 0,
            .color = color };
    if (it != m_text_buffers.end()) {
        buffer.m_handle = it - m_text_buffers.begin();
        *it = std::move(buffer_impl);
    }
    else {
        buffer.m_handle = m_text_buffers.size();
        m_text_buffers.emplace_back(std::move(buffer_impl));
    }
    set_text_buffer_scale(buffer, scale);
    set_text_buffer_translation(buffer, pos);
    set_text_buffer_color(buffer, color);
    update_text_buffer(buffer, text);
    return buffer;
}

void TextPipeline::set_text_buffer_color(const TextBuffer& buffer, const nnm::Vector3f color)
{
    VV_DEB_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to set color on invalid text buffer")
    // ReSharper disable once CppUseStructuredBinding
    TextBufferImpl& buffer_impl = *m_text_buffers[buffer.m_handle];
    buffer_impl.color = color;
    // ReSharper disable once CppUseStructuredBinding
    for (RenderGlyph& glyph : buffer_impl.render_glyphs) {
        glyph.ubo.update(m_text_color_location, color);
    }
}
float TextPipeline::text_buffer_width(const TextBuffer& buffer) const
{
    VV_DEB_ASSERT(buffer.m_valid, "[Text Pipeline] Attempt to get width on invalid text buffer")
    // ReSharper disable once CppUseStructuredBinding
    const TextBufferImpl& buffer_impl = *m_text_buffers[buffer.m_handle];
    nnm::Vector2 pos = buffer_impl.translation;
    // ReSharper disable once CppUseStructuredBinding
    for (const RenderGlyph& glyph : buffer_impl.render_glyphs) {
        VV_DEB_ASSERT(m_font_chars.contains(glyph.character), "[Text Pipeline] Attempt to access invalid character")
        // ReSharper disable once CppUseStructuredBinding
        const FontChar& font_char = m_font_chars.at(glyph.character);
        pos.x += nnm::floor(static_cast<float>(font_char.advance) / 64.0f) * glyph.scale;
    }
    return pos.x - buffer_impl.translation.x;
}
