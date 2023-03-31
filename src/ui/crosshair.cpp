#include "crosshair.hpp"

Crosshair::Crosshair(UIPipeline& pipeline)
    : m_pipeline(&pipeline)
    , m_uniform_data(pipeline.create_uniform_data())
    , m_texture(pipeline.renderer().create_texture("../res/cross.png"))
{
    auto transform = mve::Matrix4::identity().translate(
        { pipeline.renderer().extent().x / 2.0f, pipeline.renderer().extent().y / 2.0f, 0.0f });
    m_uniform_data.buffer.update(pipeline.model_location(), transform);
    m_uniform_data.descriptor_set.write_binding(pipeline.texture_binding(), m_texture);

    const float cross_scale = 25.0f;
    const mve::Vector3 cross_color { 0.75f, 0.75f, 0.75f };

    mve::VertexData cross_data(pipeline.vertex_layout());
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

    m_vertex_buffer = pipeline.renderer().create_vertex_buffer(cross_data);
    m_index_buffer = pipeline.renderer().create_index_buffer({ 0, 3, 2, 0, 2, 1 });
}

void Crosshair::draw() const
{
    m_pipeline->draw(m_uniform_data.descriptor_set, m_vertex_buffer, m_index_buffer);
}
void Crosshair::resize()
{
    auto transform = mve::Matrix4::identity().translate(
        { m_pipeline->renderer().extent().x / 2.0f, m_pipeline->renderer().extent().y / 2.0f, 0.0f });
    m_uniform_data.buffer.update(m_pipeline->model_location(), transform);
}
