#include "ui_pipeline.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "logger.hpp"
#include "mve/math/math.hpp"

UIPipeline::UIPipeline(mve::Renderer& renderer)
    : m_renderer(&renderer)
    , m_vertex_shader(mve::Shader("../res/bin/shader/ui.vert.spv"))
    , m_fragment_shader(mve::Shader("../res/bin/shader/ui.frag.spv"))
    , m_graphics_pipeline(renderer.create_graphics_pipeline(m_vertex_shader, m_fragment_shader, vertex_layout(), false))
    , m_global_ubo(renderer.create_uniform_buffer(m_vertex_shader.descriptor_set(0).binding(0)))
    , m_global_descriptor_set(renderer.create_descriptor_set(m_graphics_pipeline, m_vertex_shader.descriptor_set(0)))
    , m_view_location(m_vertex_shader.descriptor_set(0).binding(0).member("view").location())
    , m_proj_location(m_vertex_shader.descriptor_set(0).binding(0).member("proj").location())
    , m_text_vert_shader("../res/bin/shader/text.vert.spv")
    , m_text_frag_shader("../res/bin/shader/text.frag.spv")
    , m_text_pipeline(renderer.create_graphics_pipeline(m_text_vert_shader, m_text_frag_shader, c_text_vertex_layout))
    , m_text_ubo(renderer.create_uniform_buffer(m_text_vert_shader.descriptor_set(0).binding(0)))
    , m_text_descriptor_set(m_text_pipeline.create_descriptor_set(m_text_vert_shader.descriptor_set(0)))
{
    m_global_descriptor_set.write_binding(m_vertex_shader.descriptor_set(0).binding(0), m_global_ubo);
    m_text_descriptor_set.write_binding(m_vertex_shader.descriptor_set(0).binding(0), m_text_ubo);

    mve::VertexData text_data(c_text_vertex_layout);
    text_data.push_back({ 0.0f, -1.0f, 0.0f });
    text_data.push_back({ 0.0f, 0.0f });
    text_data.push_back({ 1.0f, -1.0f, 0.0f });
    text_data.push_back({ 1.0f, 0.0f });
    text_data.push_back({ 1.0f, 0.0f, 0.0f });
    text_data.push_back({ 1.0f, 1.0f });
    text_data.push_back({ 0.0f, 0.0f, 0.0f });
    text_data.push_back({ 0.0f, 1.0f });
    m_text_vertex_buffer = renderer.create_vertex_buffer(text_data);
    m_text_index_buffer = renderer.create_index_buffer({ 0, 3, 2, 0, 2, 1 });

    mve::Matrix4 camera;
    camera = camera.translate({ 0.0f, 0.0f, 0.0f });
    mve::Matrix4 view = camera.inverse().transpose();
    m_global_ubo.update(m_view_location, view);
    m_text_ubo.update(m_text_vert_shader.descriptor_set(0).binding(0).member("view").location(), view);

    // Font stuff
    FT_Library ft;
    auto ft_init_result = FT_Init_FreeType(&ft);
    if (ft_init_result != 0) {
        throw std::runtime_error("[UI Renderer] Failed to init FreeType");
    }
    FT_Face font_face;
    auto new_face_result = FT_New_Face(ft, "../res/karma_suture.otf", 0, &font_face);
    //    auto new_face_result = FT_New_Face(ft, "../res/epilepsy_sans.ttf", 0, &font_face);
    //    auto new_face_result = FT_New_Face(ft, "../res/arial.ttf", 0, &font_face);
    if (new_face_result != 0) {
        throw std::runtime_error("[UI Renderer] Failed to load font");
    }
    FT_Set_Pixel_Sizes(font_face, 0, 36);

    for (unsigned char c = 0; c < 128; c++) {
        auto load_char_result = FT_Load_Char(font_face, c, FT_LOAD_RENDER);
        if (load_char_result != 0) {
            throw std::runtime_error("[UI Renderer] Failed to load glyph");
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

        FontChar font_char { .texture = std::move(texture),
                             .size = { static_cast<int>(font_face->glyph->bitmap.width),
                                       static_cast<int>(font_face->glyph->bitmap.rows) },
                             .bearing = { font_face->glyph->bitmap_left, font_face->glyph->bitmap_top },
                             .advance = static_cast<uint32_t>(font_face->glyph->advance.x) };
        m_font_chars.insert({ c, std::move(font_char) });
    }

    FT_Done_Face(font_face);
    FT_Done_FreeType(ft);
}

void UIPipeline::resize()
{
    mve::Matrix4 proj = mve::ortho(
        -static_cast<float>(m_renderer->extent().x) / 2.0f,
        static_cast<float>(m_renderer->extent().x) / 2.0f,
        -static_cast<float>(m_renderer->extent().y) / 2.0f,
        static_cast<float>(m_renderer->extent().y) / 2.0f,
        -1000.0f,
        1000.0f);
    m_global_ubo.update(m_proj_location, proj);
    m_text_ubo.update(m_text_vert_shader.descriptor_set(0).binding(0).member("proj").location(), proj);
    if (m_show_debug) {
        update_debug_glyphs();
    }
}

void UIPipeline::draw()
{
    m_renderer->bind_graphics_pipeline(m_text_pipeline);
    m_renderer->bind_vertex_buffer(m_text_vertex_buffer);
    if (m_show_debug) {
        for (const RenderGlyph& glyph : m_debug_glyphs) {
            if (glyph.is_valid) {
                m_renderer->bind_descriptor_sets(m_text_descriptor_set, glyph.descriptor_set);
                m_renderer->draw_index_buffer(m_text_index_buffer);
            }
        }
    }
}
void UIPipeline::update_framebuffer_texture(const mve::Texture& texture, mve::Vector2i size)
{
    m_global_descriptor_set.write_binding(m_fragment_shader.descriptor_set(0).binding(1), texture);
    mve::VertexData world_data(vertex_layout());
    world_data.push_back(mve::Vector3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    world_data.push_back({ 1, 1, 1 });
    world_data.push_back({ 0.0f, 0.0f });
    world_data.push_back(mve::Vector3(0.5f * size.x, -0.5f * size.y, 0.0f));
    world_data.push_back({ 1, 1, 1 });
    world_data.push_back({ 1.0f, 0.0f });
    world_data.push_back(mve::Vector3(0.5f * size.x, 0.5f * size.y, 0.0f));
    world_data.push_back({ 1, 1, 1 });
    world_data.push_back({ 1.0f, 1.0f });
    world_data.push_back(mve::Vector3(-0.5f * size.x, 0.5f * size.y, 0.0f));
    world_data.push_back({ 1, 1, 1 });
    world_data.push_back({ 0.0f, 1.0f });
    if (!m_world.has_value()) {
        World world { .vertex_buffer = m_renderer->create_vertex_buffer(world_data),
                      .index_buffer = m_renderer->create_index_buffer({ 0, 3, 2, 0, 2, 1 }),
                      .descriptor_set = m_graphics_pipeline.create_descriptor_set(m_vertex_shader.descriptor_set(1)),
                      .uniform_buffer = m_renderer->create_uniform_buffer(m_vertex_shader.descriptor_set(1).binding(0)),
                      .model_location = m_vertex_shader.descriptor_set(1).binding(0).member("model").location() };
        world.descriptor_set.write_binding(m_vertex_shader.descriptor_set(1).binding(0), world.uniform_buffer);
        world.descriptor_set.write_binding(m_fragment_shader.descriptor_set(1).binding(1), texture);
        world.uniform_buffer.update(world.model_location, mve::Matrix4::identity());
        m_world = std::move(world);
    }
    else {
        m_world->vertex_buffer = m_renderer->create_vertex_buffer(world_data);
        m_world->descriptor_set.write_binding(m_fragment_shader.descriptor_set(1).binding(1), texture);
    }
    m_global_ubo.update(
        m_vertex_shader.descriptor_set(0).binding(0).member("world_size").location(), mve::Vector2(size));
}

void UIPipeline::update_fps(int value)
{
    m_fps_value = value;
    update_debug_glyphs();
}

void UIPipeline::update_debug_glyphs()
{
    if (!m_show_debug) {
        return;
    }
    for (RenderGlyph& glyph : m_debug_glyphs) {
        glyph.is_valid = false;
    }
    char buffer[100];
    std::snprintf(buffer, sizeof(buffer), "fps: %d", m_fps_value);
    mve::Vector2i extent = m_renderer->extent();
    float scale = 0.8f;
    float x = -extent.x / 2.0f + 8.0f * scale;
    float y = extent.y / 2.0f - 35.0f * scale;
    add_glyphs(m_debug_glyphs, std::string(buffer), { x, y }, scale);
    y -= 42.0f * scale;
    std::snprintf(buffer, sizeof(buffer), "ms: %.1f", 1000.0f / m_fps_value);
    add_glyphs(m_debug_glyphs, std::string(buffer), { x, y }, scale);
    y -= 42.0f * scale;
    std::snprintf(buffer, sizeof(buffer), "gpu: %s", m_gpu_name.c_str());
    add_glyphs(m_debug_glyphs, std::string(buffer), { x, y }, scale);
    y -= 42.0f * scale;
#ifdef NDEBUG
    std::snprintf(buffer, sizeof(buffer), "build: optimized");
#else
    std::snprintf(buffer, sizeof(buffer), "build: debug");
#endif
    add_glyphs(m_debug_glyphs, std::string(buffer), { x, y }, scale);
    y -= 42.0f * scale;
    std::snprintf(
        buffer,
        sizeof(buffer),
        "block: [%d, %d, %d]",
        m_player_block_pos.x,
        m_player_block_pos.y,
        m_player_block_pos.z);
    add_glyphs(m_debug_glyphs, std::string(buffer), { x, y }, scale);
    y -= 42.0f * scale;
    std::snprintf(
        buffer,
        sizeof(buffer),
        "chunk: [%d, %d, %d]",
        m_player_chunk_pos.x,
        m_player_chunk_pos.y,
        m_player_chunk_pos.z);
    add_glyphs(m_debug_glyphs, std::string(buffer), { x, y }, scale);
}
void UIPipeline::add_glyphs(std::vector<RenderGlyph>& glyphs, const std::string& text, mve::Vector2 pos, float scale)
{
    float x = pos.x;
    float y = pos.y;
    for (auto c = text.begin(); c != text.end(); c++) {
        if (!m_font_chars.contains(*c)) {
            LOG->info("INVALID CHAR: {}", static_cast<uint32_t>(*c));
            continue;
        }
        const FontChar& font_char = m_font_chars.at(*c);
        float x_pos = x + font_char.bearing.x * scale;
        float y_pos = y - (font_char.size.y - font_char.bearing.y) * scale;

        float w = font_char.size.x * scale;
        float h = font_char.size.y * scale;

        mve::Matrix4 model = mve::Matrix4::identity();
        model = model.scale({ w, h, 1.0f });
        model = model.translate({ x_pos, -y_pos, 0.0f });

        auto it = std::find_if(m_debug_glyphs.begin(), m_debug_glyphs.end(), [](const RenderGlyph& glyph) {
            return !glyph.is_valid;
        });
        if (it != m_debug_glyphs.end()) {
            it->ubo.update(m_text_vert_shader.descriptor_set(1).binding(0).member("model").location(), model);
            it->ubo.update(
                m_text_vert_shader.descriptor_set(1).binding(0).member("text_color").location(),
                mve::Vector3(0.0f, 0.0f, 0.0f));
            it->descriptor_set.write_binding(m_text_frag_shader.descriptor_set(1).binding(1), font_char.texture);
            it->is_valid = true;
        }
        else {
            mve::UniformBuffer glyph_ubo
                = m_renderer->create_uniform_buffer(m_text_vert_shader.descriptor_set(1).binding(0));
            mve::DescriptorSet glyph_descriptor_set
                = m_text_pipeline.create_descriptor_set(m_text_vert_shader.descriptor_set(1));
            glyph_descriptor_set.write_binding(m_text_vert_shader.descriptor_set(1).binding(0), glyph_ubo);
            RenderGlyph render_glyph {
                .is_valid = true, .ubo = std::move(glyph_ubo), .descriptor_set = std::move(glyph_descriptor_set)
            };

            render_glyph.ubo.update(m_text_vert_shader.descriptor_set(1).binding(0).member("model").location(), model);
            render_glyph.ubo.update(
                m_text_vert_shader.descriptor_set(1).binding(0).member("text_color").location(),
                mve::Vector3(0.0f, 0.0f, 0.0f));
            render_glyph.descriptor_set.write_binding(
                m_text_frag_shader.descriptor_set(1).binding(1), font_char.texture);

            glyphs.push_back(std::move(render_glyph));
        }

        x += mve::floor(font_char.advance / 64.0f) * scale;
    }
}
UIUniformData UIPipeline::create_uniform_data()
{
    UIUniformData data
        = { .descriptor_set = m_graphics_pipeline.create_descriptor_set(m_vertex_shader.descriptor_set(1)),
            .buffer = m_renderer->create_uniform_buffer(m_vertex_shader.descriptor_set(1).binding(0)) };
    data.descriptor_set.write_binding(m_vertex_shader.descriptor_set(1).binding(0), data.buffer);
    return data;
}

void UIPipeline::bind() const
{
    m_renderer->bind_graphics_pipeline(m_graphics_pipeline);
}
void UIPipeline::draw(
    const mve::DescriptorSet& descriptor_set,
    const mve::VertexBuffer& vertex_buffer,
    const mve::IndexBuffer& index_buffer) const
{
    m_renderer->bind_graphics_pipeline(m_graphics_pipeline);
    m_renderer->bind_descriptor_sets(m_global_descriptor_set, descriptor_set);
    m_renderer->bind_vertex_buffer(vertex_buffer);
    m_renderer->draw_index_buffer(index_buffer);
}
void UIPipeline::draw_world() const
{
    m_renderer->bind_graphics_pipeline(m_graphics_pipeline);
    if (m_world.has_value()) {
        m_renderer->bind_descriptor_sets(m_global_descriptor_set, m_world->descriptor_set);
        m_renderer->bind_vertex_buffer(m_world->vertex_buffer);
        m_renderer->draw_index_buffer(m_world->index_buffer);
    }
}
