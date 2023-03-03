#include "ui_renderer.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "chunk_data.hpp"
#include "common.hpp"
#include "logger.hpp"
#include "mve/math/math.hpp"

UIRenderer::UIRenderer(mve::Renderer& renderer)
    : m_renderer(&renderer)
    , m_vertex_shader(mve::Shader("../res/bin/shader/ui.vert.spv"))
    , m_fragment_shader(mve::Shader("../res/bin/shader/ui.frag.spv"))
    , m_graphics_pipeline(renderer.create_graphics_pipeline(m_vertex_shader, m_fragment_shader, c_vertex_layout, false))
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

    mve::VertexData cross_data(c_vertex_layout);

    //    const float cross_scale = 300.0f;
    const float cross_scale = 25.0f;

    const mve::Vector3 cross_color { 0.75f, 0.75f, 0.75f };

    cross_data.push_back(mve::Vector3(-0.5f, -0.5f, 0.0f) * cross_scale);
    cross_data.push_back(cross_color);
    cross_data.push_back({ 0.0f, 0.0f });
    cross_data.push_back(mve::Vector3(0.5f, -0.5f, 0.0f) * cross_scale);
    cross_data.push_back(cross_color);
    cross_data.push_back({ 1.0f, 0.0f });
    cross_data.push_back(mve::Vector3(0.5f, 0.5f, 0.0f) * cross_scale);
    cross_data.push_back(cross_color);
    cross_data.push_back({ 1.0f, 1.0f });
    cross_data.push_back(mve::Vector3(-0.5f, 0.5f, 0.0f) * cross_scale);
    cross_data.push_back(cross_color);
    cross_data.push_back({ 0.0f, 1.0f });

    UIMesh cross { .vertex_buffer = renderer.create_vertex_buffer(cross_data),
                   .index_buffer = renderer.create_index_buffer({ 0, 3, 2, 0, 2, 1 }),
                   .texture = renderer.create_texture("../res/cross.png"),
                   .descriptor_set = m_graphics_pipeline.create_descriptor_set(m_vertex_shader.descriptor_set(1)),
                   .uniform_buffer = renderer.create_uniform_buffer(m_vertex_shader.descriptor_set(1).binding(0)),
                   .model_location = m_vertex_shader.descriptor_set(1).binding(0).member("model").location() };
    cross.descriptor_set.write_binding(m_vertex_shader.descriptor_set(1).binding(0), cross.uniform_buffer);
    cross.descriptor_set.write_binding(m_fragment_shader.descriptor_set(1).binding(1), cross.texture);
    cross.uniform_buffer.update(cross.model_location, mve::Matrix4::identity());
    m_cross = std::move(cross);

    mve::VertexData hotbar_data = create_hotbar_vertex_data();
    UIMesh hotbar
        = { .vertex_buffer = m_renderer->create_vertex_buffer(hotbar_data),
            .index_buffer = m_renderer->create_index_buffer({ 0, 3, 2, 0, 2, 1 }),
            .texture = m_renderer->create_texture("../res/hotbar.png"),
            .descriptor_set = m_graphics_pipeline.create_descriptor_set(m_vertex_shader.descriptor_set(1)),
            .uniform_buffer = m_renderer->create_uniform_buffer(m_vertex_shader.descriptor_set(1).binding(0)),
            .model_location = m_vertex_shader.descriptor_set(1).binding(0).member("model").location() };
    hotbar.descriptor_set.write_binding(m_vertex_shader.descriptor_set(1).binding(0), hotbar.uniform_buffer);
    hotbar.descriptor_set.write_binding(m_fragment_shader.descriptor_set(1).binding(1), hotbar.texture);
    hotbar.uniform_buffer.update(hotbar.model_location, mve::Matrix4::identity());
    m_hotbar = std::move(hotbar);

    mve::VertexData hotbar_select_data = create_hotbar_select_vertex_data();
    UIMesh hotbar_select {
        .vertex_buffer = m_renderer->create_vertex_buffer(hotbar_select_data),
        .index_buffer = m_renderer->create_index_buffer({ 0, 3, 2, 0, 2, 1 }),
        .texture = m_renderer->create_texture("../res/hotbar_select.png"),
        .descriptor_set = m_graphics_pipeline.create_descriptor_set(m_vertex_shader.descriptor_set(1)),
        .uniform_buffer = m_renderer->create_uniform_buffer(m_vertex_shader.descriptor_set(1).binding(0)),
        .model_location = m_vertex_shader.descriptor_set(1).binding(0).member("model").location()
    };
    hotbar_select.descriptor_set.write_binding(
        m_vertex_shader.descriptor_set(1).binding(0), hotbar_select.uniform_buffer);
    hotbar_select.descriptor_set.write_binding(m_fragment_shader.descriptor_set(1).binding(1), hotbar_select.texture);
    hotbar_select.uniform_buffer.update(hotbar_select.model_location, mve::Matrix4::identity());
    m_hotbar_select = std::move(hotbar_select);

    // Font stuff
    FT_Library ft;
    auto ft_init_result = FT_Init_FreeType(&ft);
    if (ft_init_result != 0) {
        throw std::runtime_error("[UI Renderer] Failed to init FreeType");
    }
    FT_Face font_face;
    auto new_face_result = FT_New_Face(ft, "../res/epilepsy_sans.ttf", 0, &font_face);
    //    auto new_face_result = FT_New_Face(ft, "../res/arial.ttf", 0, &font_face);
    if (new_face_result != 0) {
        throw std::runtime_error("[UI Renderer] Failed to load font");
    }
    FT_Set_Pixel_Sizes(font_face, 0, 48);

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

    std::string text = "/gamemode creative";
    float x = 20.0f;
    float y = 0.0f;
    float scale = 1.0f;
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

        mve::UniformBuffer glyph_ubo = renderer.create_uniform_buffer(m_text_vert_shader.descriptor_set(1).binding(0));
        mve::DescriptorSet glyph_descriptor_set
            = m_text_pipeline.create_descriptor_set(m_text_vert_shader.descriptor_set(1));
        glyph_descriptor_set.write_binding(m_text_vert_shader.descriptor_set(1).binding(0), glyph_ubo);
        RenderGlyph render_glyph { .ubo = std::move(glyph_ubo), .descriptor_set = std::move(glyph_descriptor_set) };

        render_glyph.ubo.update(m_text_vert_shader.descriptor_set(1).binding(0).member("model").location(), model);
        render_glyph.ubo.update(
            m_text_vert_shader.descriptor_set(1).binding(0).member("text_color").location(),
            mve::Vector3(0.0f, 0.0f, 0.0f));
        render_glyph.descriptor_set.write_binding(m_text_frag_shader.descriptor_set(1).binding(1), font_char.texture);

        m_render_glyphs.push_back(std::move(render_glyph));

        x += mve::floor(font_char.advance / 64.0f) * scale;
    }
}

void UIRenderer::resize()
{
    mve::Matrix4 proj = mve::ortho(
        -static_cast<float>(m_renderer->extent().x) / 2.0f,
        static_cast<float>(m_renderer->extent().x) / 2.0f,
        -static_cast<float>(m_renderer->extent().y) / 2.0f,
        static_cast<float>(m_renderer->extent().y) / 2.0f,
        -1000.0f,
        1000.0f);
    mve::Vector3 hotbar_scale(
        (mve::Vector3(m_renderer->extent().x) / 1000.0f).clamp(mve::Vector3(0.1), mve::Vector3(1.0f)));
    mve::Vector3 hotbar_translation(0, static_cast<float>(m_renderer->extent().y) * 0.5f, 0);
    m_global_ubo.update(m_proj_location, proj);
    m_text_ubo.update(m_text_vert_shader.descriptor_set(0).binding(0).member("proj").location(), proj);
    if (m_hotbar.has_value()) {
        m_hotbar->uniform_buffer.update(
            m_hotbar->model_location, mve::Matrix4::identity().scale(hotbar_scale).translate(hotbar_translation));
    }
    set_hotbar_select(m_current_hotbar_select);
    for (auto& [pos, block] : m_hotbar_blocks) {
        if (block.has_value()) {
            int first_offset_x = -20 * 5 * 4;
            block->uniform_buffer.update(
                block->model_location,
                mve::Matrix4::identity()
                    .scale(hotbar_scale)
                    .translate(
                        hotbar_translation + mve::Vector3(first_offset_x + (pos * 20 * 5), -5 * 4, 0) * hotbar_scale));
        }
    }
}

void UIRenderer::draw()
{
    m_renderer->bind_graphics_pipeline(m_graphics_pipeline);
    if (m_world.has_value()) {
        m_renderer->bind_descriptor_sets(m_global_descriptor_set, m_world->descriptor_set);
        m_renderer->bind_vertex_buffer(m_world->vertex_buffer);
        m_renderer->draw_index_buffer(m_world->index_buffer);
    }
    if (m_cross.has_value()) {
        m_renderer->bind_descriptor_sets(m_global_descriptor_set, m_cross.value().descriptor_set);
        m_renderer->bind_vertex_buffer(m_cross.value().vertex_buffer);
        m_renderer->draw_index_buffer(m_cross.value().index_buffer);
    }
    if (m_hotbar) {
        m_renderer->bind_descriptor_sets(m_global_descriptor_set, m_hotbar->descriptor_set);
        m_renderer->bind_vertex_buffer(m_hotbar->vertex_buffer);
        m_renderer->draw_index_buffer(m_hotbar->index_buffer);
    }
    for (const auto& [pos, block] : m_hotbar_blocks) {
        m_renderer->bind_descriptor_sets(m_global_descriptor_set, block->descriptor_set);
        m_renderer->bind_vertex_buffer(block->vertex_buffer);
        m_renderer->draw_index_buffer(block->index_buffer);
    }
    if (m_hotbar_select) {
        m_renderer->bind_descriptor_sets(m_global_descriptor_set, m_hotbar_select->descriptor_set);
        m_renderer->bind_vertex_buffer(m_hotbar_select->vertex_buffer);
        m_renderer->draw_index_buffer(m_hotbar_select->index_buffer);
    }
    //    m_renderer->bind_graphics_pipeline(m_text_pipeline);
    //    m_renderer->bind_vertex_buffer(m_text_vertex_buffer);
    //    for (const RenderGlyph& glyph : m_render_glyphs) {
    //        m_renderer->bind_descriptor_sets(m_text_descriptor_set, glyph.descriptor_set);
    //        m_renderer->draw_index_buffer(m_text_index_buffer);
    //    }
}
void UIRenderer::update_framebuffer_texture(const mve::Texture& texture, mve::Vector2i size)
{
    m_global_descriptor_set.write_binding(m_fragment_shader.descriptor_set(0).binding(1), texture);
    mve::VertexData world_data(c_vertex_layout);
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
    if (m_cross.has_value()) {
        m_cross->uniform_buffer.update(
            m_vertex_shader.descriptor_set(1).binding(0).member("scale").location(),
            mve::Vector2(25.0f / size.x, 25.0f / size.y));
        m_cross->uniform_buffer.update(
            m_vertex_shader.descriptor_set(1).binding(0).member("contrast").location(), 1.0f);
    }
}

mve::VertexData UIRenderer::create_hotbar_vertex_data() const
{
    mve::Vector2 size { 910, 110 };
    mve::VertexData data(c_vertex_layout);
    data.push_back(mve::Vector3(-0.5f * size.x, -1.0f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 0.0f, 0.0f });
    data.push_back(mve::Vector3(0.5f * size.x, -1.0f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 1.0f, 0.0f });
    data.push_back(mve::Vector3(0.5f * size.x, 0.0f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 1.0f, 1.0f });
    data.push_back(mve::Vector3(-0.5f * size.x, 0.0f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 0.0f, 1.0f });

    return data;
}
mve::VertexData UIRenderer::create_hotbar_select_vertex_data() const
{
    mve::Vector2 size { 24 * 5, 23 * 5 };
    mve::VertexData data(c_vertex_layout);
    data.push_back(mve::Vector3(-0.5f * size.x, -1.0f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 0.0f, 0.0f });
    data.push_back(mve::Vector3(0.5f * size.x, -1.0f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 1.0f, 0.0f });
    data.push_back(mve::Vector3(0.5f * size.x, 0.0f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 1.0f, 1.0f });
    data.push_back(mve::Vector3(-0.5f * size.x, 0.0f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 0.0f, 1.0f });

    return data;
}

void UIRenderer::set_hotbar_select(int pos)
{
    m_current_hotbar_select = pos;
    mve::Vector3 hotbar_scale(
        (mve::Vector3(m_renderer->extent().x) / 1000.0f).clamp(mve::Vector3(0.1), mve::Vector3(1.0f)));
    mve::Vector3 hotbar_translation(0, static_cast<float>(m_renderer->extent().y) * 0.5f, 0);
    int first_offset_x = -20 * 5 * 4;
    if (m_hotbar_select.has_value()) {
        m_hotbar_select->uniform_buffer.update(
            m_hotbar_select->model_location,
            mve::Matrix4::identity()
                .scale(hotbar_scale)
                .translate(hotbar_translation + mve::Vector3(first_offset_x + (pos * 20 * 5), 0, 0) * hotbar_scale));
    }
}
UIRenderer::MeshData UIRenderer::create_block_face_data(uint8_t block_type) const
{
    mve::Vector2 size(14 * 5);
    MeshData data;
    data.vertices.push_back(mve::Vector3(-0.5f * size.x, -1.0f * size.y, 0.0f));
    data.vertices.push_back(mve::Vector3(0.5f * size.x, -1.0f * size.y, 0.0f));
    data.vertices.push_back(mve::Vector3(0.5f * size.x, 0.0f * size.y, 0.0f));
    data.vertices.push_back(mve::Vector3(-0.5f * size.x, 0.0f * size.y, 0.0f));
    QuadUVs quad_uvs = uvs_from_atlas({ 4, 4 }, block_uv(block_type, Direction::front));
    data.uvs.push_back(quad_uvs.top_left);
    data.uvs.push_back(quad_uvs.top_right);
    data.uvs.push_back(quad_uvs.bottom_right);
    data.uvs.push_back(quad_uvs.bottom_left);
    data.colors.push_back({ 1, 1, 1 });
    data.colors.push_back({ 1, 1, 1 });
    data.colors.push_back({ 1, 1, 1 });
    data.colors.push_back({ 1, 1, 1 });
    data.indices = { 0, 3, 2, 0, 2, 1 };
    return data;
}

void UIRenderer::set_hotbar_block(int pos, uint8_t block_type)
{
    MeshData block_data = create_block_face_data(block_type);
    mve::VertexData block_vertex_data(c_vertex_layout);
    for (size_t i = 0; i < block_data.vertices.size(); i++) {
        block_vertex_data.push_back(block_data.vertices[i]);
        block_vertex_data.push_back(block_data.colors[i]);
        block_vertex_data.push_back(block_data.uvs[i]);
    }
    UIMesh block { .vertex_buffer = m_renderer->create_vertex_buffer(block_vertex_data),
                   .index_buffer = m_renderer->create_index_buffer(block_data.indices),
                   .texture = m_renderer->create_texture("../res/atlas.png"),
                   .descriptor_set = m_graphics_pipeline.create_descriptor_set(m_vertex_shader.descriptor_set(1)),
                   .uniform_buffer = m_renderer->create_uniform_buffer(m_vertex_shader.descriptor_set(1).binding(0)),
                   .model_location = m_vertex_shader.descriptor_set(1).binding(0).member("model").location() };
    block.descriptor_set.write_binding(m_vertex_shader.descriptor_set(1).binding(0), block.uniform_buffer);
    block.descriptor_set.write_binding(m_fragment_shader.descriptor_set(1).binding(1), block.texture);
    block.uniform_buffer.update(block.model_location, mve::Matrix4::identity());
    m_hotbar_blocks[pos] = std::move(block);
}
