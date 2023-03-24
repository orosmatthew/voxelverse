#include "hotbar.hpp"

#include "../common.hpp"
#include "../logger.hpp"

Hotbar::Hotbar(UIPipeline& ui_pipeline)
    : m_ui_pipeline(&ui_pipeline)
    , m_model_location(m_ui_pipeline->model_location())
    , m_texture_binding(m_ui_pipeline->texture_binding())
    , m_hotbar({ .uniform_data = m_ui_pipeline->create_uniform_data() })
    , m_select({ .uniform_data = m_ui_pipeline->create_uniform_data() })
    , m_renderer_extent(ui_pipeline.renderer().extent())
    , m_select_pos(0)
    , m_atlas_texture(ui_pipeline.renderer(), "../res/atlas.png")
{
    mve::Vector2 size { 910, 110 };
    mve::VertexData vertex_data(m_ui_pipeline->vertex_layout());
    vertex_data.push_back(mve::Vector3(-0.5f * size.x, -1.0f * size.y, 0.0f));
    vertex_data.push_back({ 1, 1, 1 });
    vertex_data.push_back({ 0.0f, 0.0f });
    vertex_data.push_back(mve::Vector3(0.5f * size.x, -1.0f * size.y, 0.0f));
    vertex_data.push_back({ 1, 1, 1 });
    vertex_data.push_back({ 1.0f, 0.0f });
    vertex_data.push_back(mve::Vector3(0.5f * size.x, 0.0f * size.y, 0.0f));
    vertex_data.push_back({ 1, 1, 1 });
    vertex_data.push_back({ 1.0f, 1.0f });
    vertex_data.push_back(mve::Vector3(-0.5f * size.x, 0.0f * size.y, 0.0f));
    vertex_data.push_back({ 1, 1, 1 });
    vertex_data.push_back({ 0.0f, 1.0f });

    m_hotbar_texture = ui_pipeline.renderer().create_texture("../res/hotbar.png");
    m_hotbar.vertex_buffer = ui_pipeline.renderer().create_vertex_buffer(vertex_data);
    m_hotbar.index_buffer = ui_pipeline.renderer().create_index_buffer({ 0, 3, 2, 0, 2, 1 });
    m_hotbar.uniform_data.descriptor_set.write_binding(m_texture_binding, m_hotbar_texture);
    m_hotbar.uniform_data.buffer.update(m_model_location, mve::Matrix4::identity());

    mve::Vector2 select_size { 24 * 5, 23 * 5 };
    mve::VertexData select_vertex_data(m_ui_pipeline->vertex_layout());
    select_vertex_data.push_back(mve::Vector3(-0.5f * select_size.x, -1.0f * select_size.y, 0.0f));
    select_vertex_data.push_back({ 1, 1, 1 });
    select_vertex_data.push_back({ 0.0f, 0.0f });
    select_vertex_data.push_back(mve::Vector3(0.5f * select_size.x, -1.0f * select_size.y, 0.0f));
    select_vertex_data.push_back({ 1, 1, 1 });
    select_vertex_data.push_back({ 1.0f, 0.0f });
    select_vertex_data.push_back(mve::Vector3(0.5f * select_size.x, 0.0f * select_size.y, 0.0f));
    select_vertex_data.push_back({ 1, 1, 1 });
    select_vertex_data.push_back({ 1.0f, 1.0f });
    select_vertex_data.push_back(mve::Vector3(-0.5f * select_size.x, 0.0f * select_size.y, 0.0f));
    select_vertex_data.push_back({ 1, 1, 1 });
    select_vertex_data.push_back({ 0.0f, 1.0f });

    m_select_texture = ui_pipeline.renderer().create_texture("../res/hotbar_select.png");
    m_select.vertex_buffer = ui_pipeline.renderer().create_vertex_buffer(select_vertex_data);
    m_select.index_buffer = ui_pipeline.renderer().create_index_buffer({ 0, 3, 2, 0, 2, 1 });
    m_select.uniform_data.descriptor_set.write_binding(m_texture_binding, m_select_texture);
    m_select.uniform_data.buffer.update(m_model_location, mve::Matrix4::identity());
}

void Hotbar::resize(const mve::Vector2i& extent)
{
    m_renderer_extent = extent;
    m_hotbar.uniform_data.buffer.update(
        m_model_location, mve::Matrix4::identity().scale(scale()).translate(translation()));
    update_hotbar_select(m_select_pos);
    for (auto& [pos, item] : m_items) {
        if (item.has_value()) {
            int first_offset_x = -20 * 5 * 4;
            item->element.uniform_data.buffer.update(
                m_model_location,
                mve::Matrix4::identity().scale(scale()).translate(
                    translation() + mve::Vector3(first_offset_x + (pos * 20 * 5), -5 * 4, 0) * scale()));
        }
    }
}

void Hotbar::update_hotbar_select(int pos)
{
    m_select_pos = pos;
    int first_offset_x = -20 * 5 * 4;
    m_select.uniform_data.buffer.update(
        m_model_location,
        mve::Matrix4::identity().scale(scale()).translate(
            translation() + mve::Vector3(first_offset_x + (pos * 20 * 5), 0, 0) * scale()));
}
mve::Vector3 Hotbar::scale() const
{
    return (mve::Vector3(m_renderer_extent.x) / 1000.0f).clamp(mve::Vector3(0.1), mve::Vector3(1.0f));
}
mve::Vector3 Hotbar::translation() const
{
    return { 0, static_cast<float>(m_renderer_extent.y) * 0.5f, 0 };
}
std::pair<mve::VertexData, std::vector<uint32_t>> Hotbar::create_item_mesh(uint8_t block_type) const
{
    mve::Vector2 size(14 * 5);
    QuadUVs quad_uvs = uvs_from_atlas({ 4, 4 }, block_uv(block_type, Direction::front));

    mve::VertexData data(UIPipeline::vertex_layout());
    data.push_back(mve::Vector3(-0.5f * size.x, -1.0f * size.y, 0.0f));
    data.push_back({ 0.0f, 0.0f, 0.0f });
    data.push_back(quad_uvs.top_left);
    data.push_back(mve::Vector3(0.5f * size.x, -1.0f * size.y, 0.0f));
    data.push_back({ 0.0f, 0.0f, 0.0f });
    data.push_back(quad_uvs.top_right);
    data.push_back(mve::Vector3(0.5f * size.x, 0.0f * size.y, 0.0f));
    data.push_back({ 0.0f, 0.0f, 0.0f });
    data.push_back(quad_uvs.bottom_right);
    data.push_back(mve::Vector3(-0.5f * size.x, 0.0f * size.y, 0.0f));
    data.push_back({ 0.0f, 0.0f, 0.0f });
    data.push_back(quad_uvs.bottom_left);

    return { data, { 0, 3, 2, 0, 2, 1 } };
}

void Hotbar::draw() const
{
    m_ui_pipeline->draw(m_hotbar.uniform_data.descriptor_set, m_hotbar.vertex_buffer, m_hotbar.index_buffer);
    for (const auto& [pos, item] : m_items) {
        if (item.has_value()) {
            m_ui_pipeline->draw(
                item->element.uniform_data.descriptor_set, item->element.vertex_buffer, item->element.index_buffer);
        }
    }
    m_ui_pipeline->draw(m_select.uniform_data.descriptor_set, m_select.vertex_buffer, m_select.index_buffer);
}

void Hotbar::set_item(int pos, uint8_t block_type)
{
    auto [vertex_data, index_data] = create_item_mesh(block_type);
    Element element = { .uniform_data = m_ui_pipeline->create_uniform_data(),
                        .vertex_buffer = m_ui_pipeline->renderer().create_vertex_buffer(vertex_data),
                        .index_buffer = m_ui_pipeline->renderer().create_index_buffer(index_data) };
    element.uniform_data.descriptor_set.write_binding(m_texture_binding, m_atlas_texture);
    element.uniform_data.buffer.update(m_model_location, mve::Matrix4::identity());
    m_items[pos] = { block_type, std::move(element) };
}
