#include "nine_patch.hpp"

NinePatch::NinePatch(
    mve::Renderer& renderer, UIPipeline& ui_pipeline, const std::filesystem::path& img_path, NinePatchMargins margins)
    : m_margins(margins)
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

    //    m_vertex_buffer = renderer.create_vertex_buffer(vertex_data);
    //    m_index_buffer = renderer.create_index_buffer({ 0, 3, 2, 0, 2, 1 });
    //    m_texture = renderer.create_texture(img_path);
    //
    //    m_uniform_data.descriptor_set.write_binding(m_uniform_data.texture_binding, m_texture);
    //    m_uniform_data.uniform_buffer.update(m_uniform_data.model_location, mve::Matrix4::identity());
}

void NinePatch::draw(mve::Renderer& renderer, const UIPipeline& ui_pipeline)
{
    renderer.bind_descriptor_sets(ui_pipeline.global_descriptor_set(), m_uniform_data.descriptor_set);
    renderer.bind_vertex_buffer(m_vertex_buffer);
    renderer.draw_index_buffer(m_index_buffer);
}
