#include "ui_pipeline.hpp"

#include "common.hpp"

#include <nnm/nnm.hpp>

#include "../common/logger.hpp"

UIPipeline::UIPipeline(mve::Renderer& renderer)
    : m_renderer(&renderer)
    , m_vertex_shader(mve::Shader(res_path("bin/shader/ui.vert.spv")))
    , m_fragment_shader(mve::Shader(res_path("bin/shader/ui.frag.spv")))
    , m_graphics_pipeline(renderer.create_graphics_pipeline(m_vertex_shader, m_fragment_shader, vertex_layout(), false))
    , m_global_ubo(renderer.create_uniform_buffer(m_vertex_shader.descriptor_set(0).binding(0)))
    , m_global_descriptor_set(renderer.create_descriptor_set(m_graphics_pipeline, m_vertex_shader.descriptor_set(0)))
    , m_view_location(m_vertex_shader.descriptor_set(0).binding(0).member("view").location())
    , m_proj_location(m_vertex_shader.descriptor_set(0).binding(0).member("proj").location())
{
    m_global_descriptor_set.write_binding(m_vertex_shader.descriptor_set(0).binding(0), m_global_ubo);

    nnm::Transform3f camera;
    camera = camera.translate({ 0.0f, 0.0f, 0.0f });
    const nnm::Transform3f view = camera.unchecked_inverse();
    m_global_ubo.update(m_view_location, view.matrix);
}

void UIPipeline::resize()
{
    const auto proj = nnm::Transform3f::from_orthographic_right_hand_0to1(
        0.0f,
        static_cast<float>(m_renderer->extent().x),
        0.0f,
        static_cast<float>(m_renderer->extent().y),
        -1000.0f,
        1000.0f);
    m_global_ubo.update(m_proj_location, proj.matrix);
}

void UIPipeline::update_framebuffer_texture(const mve::Texture& texture, const nnm::Vector2i size)
{
    m_global_descriptor_set.write_binding(m_fragment_shader.descriptor_set(0).binding(1), texture);
    mve::VertexData world_data(vertex_layout());
    world_data.push(nnm::Vector3(0.0f, 0.0f, 0.0f));
    world_data.push({ 1, 1, 1 });
    world_data.push({ 0.0f, 0.0f });
    world_data.push(nnm::Vector3(static_cast<float>(size.x), 0.0f, 0.0f));
    world_data.push({ 1, 1, 1 });
    world_data.push({ 1.0f, 0.0f });
    world_data.push(nnm::Vector3(static_cast<float>(size.x), static_cast<float>(size.y), 0.0f));
    world_data.push({ 1, 1, 1 });
    world_data.push({ 1.0f, 1.0f });
    world_data.push(nnm::Vector3(0.0f, static_cast<float>(size.y), 0.0f));
    world_data.push({ 1, 1, 1 });
    world_data.push({ 0.0f, 1.0f });
    if (!m_world.has_value()) {
        World world { .vertex_buffer = m_renderer->create_vertex_buffer(world_data),
                      .index_buffer = m_renderer->create_index_buffer(std::array<uint32_t, 6> { 0, 3, 2, 0, 2, 1 }),
                      .descriptor_set = m_graphics_pipeline.create_descriptor_set(m_vertex_shader.descriptor_set(1)),
                      .uniform_buffer = m_renderer->create_uniform_buffer(m_vertex_shader.descriptor_set(1).binding(0)),
                      .model_location = m_vertex_shader.descriptor_set(1).binding(0).member("model").location() };
        world.descriptor_set.write_binding(m_vertex_shader.descriptor_set(1).binding(0), world.uniform_buffer);
        world.descriptor_set.write_binding(m_fragment_shader.descriptor_set(1).binding(1), texture);
        world.uniform_buffer.update(world.model_location, nnm::Matrix4f::identity());
        m_world = std::move(world);
    }
    else {
        m_world->vertex_buffer = m_renderer->create_vertex_buffer(world_data);
        m_world->descriptor_set.write_binding(m_fragment_shader.descriptor_set(1).binding(1), texture);
    }
    m_global_ubo.update(
        m_vertex_shader.descriptor_set(0).binding(0).member("world_size").location(), nnm::Vector2f(size));
}

UIUniformData UIPipeline::create_uniform_data() const
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
