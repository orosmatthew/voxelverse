#include "ui_renderer.hpp"

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

    const mve::Vector3 cross_color { 0.75f, 0.75f, 0.75f };

    cross_data.push_back({ -0.5f * 10.0f, 0.5f * 10.0f, 0.0f });
    cross_data.push_back(cross_color);
    cross_data.push_back({ 0.0f, 0.0f });

    cross_data.push_back({ 0.5f * 10.0f, 0.5f * 10.0f, 0.0f });
    cross_data.push_back(cross_color);
    cross_data.push_back({ 0.0f, 0.0f });

    cross_data.push_back({ 0.0f * 10.0f, -0.5f * 10.0f, 0.0f });
    cross_data.push_back(cross_color);
    cross_data.push_back({ 0.0f, 0.0f });

    Cross cross { .vertex_buffer = renderer.create_vertex_buffer(cross_data),
                  .descriptor_set = m_graphics_pipeline.create_descriptor_set(m_vertex_shader.descriptor_set(1)),
                  .uniform_buffer = renderer.create_uniform_buffer(m_vertex_shader.descriptor_set(1).binding(0)),
                  .model_location = m_vertex_shader.descriptor_set(1).binding(0).member("model").location() };
    cross.descriptor_set.write_binding(m_vertex_shader.descriptor_set(1).binding(0), cross.uniform_buffer);
    cross.uniform_buffer.update(cross.model_location, mve::Matrix4::identity());
    m_cross = std::move(cross);
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
}

void UIRenderer::draw()
{
    m_renderer->bind_graphics_pipeline(m_graphics_pipeline);
    if (m_cross.has_value()) {
        m_renderer->bind_descriptor_sets(m_global_descriptor_set, m_cross.value().descriptor_set);
        m_renderer->draw_vertex_buffer(m_cross.value().vertex_buffer);
    }
}
