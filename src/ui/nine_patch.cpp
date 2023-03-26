#include "nine_patch.hpp"
#include "../logger.hpp"

NinePatch::NinePatch(UIPipeline& ui_pipeline, const std::filesystem::path& img_path, NinePatchMargins margins)
    : m_pipeline(&ui_pipeline)
    , m_margins(margins)
    , m_uniform_data(ui_pipeline.create_uniform_data())
{
    mve::VertexData vertex_data(UIPipeline::vertex_layout());
    vertex_data.push_back({ 0.0f, 0.0f, 0.0f });
    vertex_data.push_back({ 1.0f, 1.0f, 1.0f });
    vertex_data.push_back({ 0.0f, 0.0f });
    vertex_data.push_back({ 1.0f, 0.0f, 0.0f });
    vertex_data.push_back({ 1.0f, 1.0f, 1.0f });
    vertex_data.push_back({ 1.0f, 0.0f });
    vertex_data.push_back({ 1.0f, 1.0f, 0.0f });
    vertex_data.push_back({ 1.0f, 1.0f, 1.0f });
    vertex_data.push_back({ 1.0f, 1.0f });
    vertex_data.push_back({ 0.0f, 1.0f, 0.0f });
    vertex_data.push_back({ 1.0f, 1.0f, 1.0f });
    vertex_data.push_back({ 0.0f, 1.0f });

    m_vertex_buffer = ui_pipeline.renderer().create_vertex_buffer(vertex_data);
    m_index_buffer = ui_pipeline.renderer().create_index_buffer({ 0, 3, 2, 0, 2, 1 });
    m_texture = ui_pipeline.renderer().create_texture(img_path);

    m_uniform_data.descriptor_set.write_binding(ui_pipeline.texture_binding(), m_texture);
    m_uniform_data.buffer.update(ui_pipeline.model_location(), mve::Matrix4::identity().scale(mve::Vector3(100.0f)));
}

void NinePatch::draw() const
{
    m_pipeline->draw(m_uniform_data.descriptor_set, m_vertex_buffer, m_index_buffer);
}
