#include "ui_renderer.hpp"

#include "logger.hpp"
#include "math/math.hpp"

UIRenderer::UIRenderer(mve::Renderer& renderer)
    : m_renderer(&renderer)
    , m_vertex_shader(mve::Shader("../res/bin/shader/ui.vert.spv", mve::ShaderType::vertex))
    , m_fragment_shader(mve::Shader("../res/bin/shader/ui.frag.spv", mve::ShaderType::fragment))
    , m_graphics_pipeline(renderer.create_graphics_pipeline(m_vertex_shader, m_fragment_shader, c_vertex_layout, false))
    , m_global_ubo(renderer.create_uniform_buffer(m_vertex_shader.descriptor_set(0).binding(0)))
    , m_global_descriptor_set(renderer.create_descriptor_set(m_graphics_pipeline, m_vertex_shader.descriptor_set(0)))
    , m_view_location(m_vertex_shader.descriptor_set(0).binding(0).member("view").location())
    , m_proj_location(m_vertex_shader.descriptor_set(0).binding(0).member("proj").location())
{
    m_global_descriptor_set.write_binding(m_vertex_shader.descriptor_set(0).binding(0), m_global_ubo);

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
    m_global_ubo.update(m_proj_location, proj);
    if (m_hotbar.has_value()) {
        m_hotbar->uniform_buffer.update(
            m_hotbar->model_location,
            mve::Matrix4::identity().translate({ 0, static_cast<float>(m_renderer->extent().y) * 0.5f - 50.0f, 0 }));
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
    mve::Vector2 size { 900, 100 };
    mve::VertexData data(c_vertex_layout);
    data.push_back(mve::Vector3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 0.0f, 0.0f });
    data.push_back(mve::Vector3(0.5f * size.x, -0.5f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 1.0f, 0.0f });
    data.push_back(mve::Vector3(0.5f * size.x, 0.5f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 1.0f, 1.0f });
    data.push_back(mve::Vector3(-0.5f * size.x, 0.5f * size.y, 0.0f));
    data.push_back({ 1, 1, 1 });
    data.push_back({ 0.0f, 1.0f });

    return data;
}
