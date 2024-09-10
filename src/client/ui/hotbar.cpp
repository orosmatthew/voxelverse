#include "hotbar.hpp"

#include "../common.hpp"

#include <ranges>

Hotbar::Hotbar(UIPipeline& ui_pipeline)
    : m_ui_pipeline(&ui_pipeline)
    , m_model_location(ui_pipeline.model_location())
    , m_texture_binding(ui_pipeline.texture_binding())
    , m_atlas_texture(ui_pipeline.renderer(), res_path("atlas.png"))
    , m_hotbar({ .uniform_data = ui_pipeline.create_uniform_data() })
    , m_select({ .uniform_data = ui_pipeline.create_uniform_data() })
    , m_renderer_extent(ui_pipeline.renderer().extent())
    , m_select_pos(0)
{
    constexpr nnm::Vector2 size { 910, 110 };
    mve::VertexData vertex_data(UIPipeline::vertex_layout());
    vertex_data.push(nnm::Vector3(-0.5f * size.x, -1.0f * size.y, 0.0f));
    vertex_data.push({ 1, 1, 1 });
    vertex_data.push({ 0.0f, 0.0f });
    vertex_data.push(nnm::Vector3(0.5f * size.x, -1.0f * size.y, 0.0f));
    vertex_data.push({ 1, 1, 1 });
    vertex_data.push({ 1.0f, 0.0f });
    vertex_data.push(nnm::Vector3(0.5f * size.x, 0.0f * size.y, 0.0f));
    vertex_data.push({ 1, 1, 1 });
    vertex_data.push({ 1.0f, 1.0f });
    vertex_data.push(nnm::Vector3(-0.5f * size.x, 0.0f * size.y, 0.0f));
    vertex_data.push({ 1, 1, 1 });
    vertex_data.push({ 0.0f, 1.0f });

    m_hotbar_texture = ui_pipeline.renderer().create_texture(res_path("hotbar.png"));
    m_hotbar.vertex_buffer = ui_pipeline.renderer().create_vertex_buffer(vertex_data);
    m_hotbar.index_buffer = ui_pipeline.renderer().create_index_buffer(std::array<uint32_t, 6> { 0, 3, 2, 0, 2, 1 });
    m_hotbar.uniform_data.descriptor_set.write_binding(m_texture_binding, m_hotbar_texture);
    m_hotbar.uniform_data.buffer.update(m_model_location, nnm::Matrix4f::identity());

    constexpr nnm::Vector2 select_size { 24 * 5, 23 * 5 };
    mve::VertexData select_vertex_data(UIPipeline::vertex_layout());
    select_vertex_data.push(nnm::Vector3(-0.5f * select_size.x, -1.0f * select_size.y, 0.0f));
    select_vertex_data.push({ 1, 1, 1 });
    select_vertex_data.push({ 0.0f, 0.0f });
    select_vertex_data.push(nnm::Vector3(0.5f * select_size.x, -1.0f * select_size.y, 0.0f));
    select_vertex_data.push({ 1, 1, 1 });
    select_vertex_data.push({ 1.0f, 0.0f });
    select_vertex_data.push(nnm::Vector3(0.5f * select_size.x, 0.0f * select_size.y, 0.0f));
    select_vertex_data.push({ 1, 1, 1 });
    select_vertex_data.push({ 1.0f, 1.0f });
    select_vertex_data.push(nnm::Vector3(-0.5f * select_size.x, 0.0f * select_size.y, 0.0f));
    select_vertex_data.push({ 1, 1, 1 });
    select_vertex_data.push({ 0.0f, 1.0f });

    m_select_texture = ui_pipeline.renderer().create_texture(res_path("hotbar_select.png"));
    m_select.vertex_buffer = ui_pipeline.renderer().create_vertex_buffer(select_vertex_data);
    m_select.index_buffer = ui_pipeline.renderer().create_index_buffer(std::array<uint32_t, 6> { 0, 3, 2, 0, 2, 1 });
    m_select.uniform_data.descriptor_set.write_binding(m_texture_binding, m_select_texture);
    m_select.uniform_data.buffer.update(m_model_location, nnm::Matrix4f::identity());
}

void Hotbar::resize(const nnm::Vector2i& extent)
{
    m_renderer_extent = extent;
    m_hotbar.uniform_data.buffer.update(
        m_model_location, nnm::Transform3f().scale(scale()).translate(translation()).matrix);
    update_hotbar_select(m_select_pos);
    for (auto& [pos, item] : m_items) {
        if (item.has_value()) {
            constexpr int first_offset_x = -20 * 5 * 4;
            item->element.uniform_data.buffer.update(
                m_model_location,
                nnm::Transform3f()
                    .scale(scale())
                    .translate(
                        translation()
                        + nnm::Vector3f(static_cast<float>(first_offset_x + pos * 20 * 5), -5 * 4, 0) * scale())
                    .matrix);
        }
    }
}

void Hotbar::update_hotbar_select(const int pos)
{
    m_select_pos = pos;
    constexpr int first_offset_x = -20 * 5 * 4;
    m_select.uniform_data.buffer.update(
        m_model_location,
        nnm::Transform3f()
            .scale(scale())
            .translate(translation() + nnm::Vector3f(static_cast<float>(first_offset_x + pos * 20 * 5), 0, 0) * scale())
            .matrix);
}
nnm::Vector3f Hotbar::scale() const
{
    return (nnm::Vector3f::all(static_cast<float>(m_renderer_extent.x)) / 1000.0f)
        .clamp(nnm::Vector3f::all(0.1), nnm::Vector3f::all(1.0f));
}
nnm::Vector3f Hotbar::translation() const
{
    return { static_cast<float>(m_renderer_extent.x) * 0.5f, static_cast<float>(m_renderer_extent.y), 0 };
}
std::pair<mve::VertexData, std::vector<uint32_t>> Hotbar::create_item_mesh(const uint8_t block_type)
{
    constexpr auto size = nnm::Vector2f::all(14 * 5);
    auto [top_left, top_right, bottom_right, bottom_left]
        = uvs_from_atlas({ 4, 4 }, block_uv(block_type, Direction::front));

    mve::VertexData data(UIPipeline::vertex_layout());
    data.push(nnm::Vector3(-0.5f * size.x, -1.0f * size.y, 0.0f));
    data.push({ 0.0f, 0.0f, 0.0f });
    data.push(top_left);
    data.push(nnm::Vector3(0.5f * size.x, -1.0f * size.y, 0.0f));
    data.push({ 0.0f, 0.0f, 0.0f });
    data.push(top_right);
    data.push(nnm::Vector3(0.5f * size.x, 0.0f * size.y, 0.0f));
    data.push({ 0.0f, 0.0f, 0.0f });
    data.push(bottom_right);
    data.push(nnm::Vector3(-0.5f * size.x, 0.0f * size.y, 0.0f));
    data.push({ 0.0f, 0.0f, 0.0f });
    data.push(bottom_left);

    return { data, { 0, 3, 2, 0, 2, 1 } };
}

void Hotbar::draw() const
{
    m_ui_pipeline->draw(m_hotbar.uniform_data.descriptor_set, m_hotbar.vertex_buffer, m_hotbar.index_buffer);
    for (const auto& item : m_items | std::views::values) {
        if (item.has_value()) {
            m_ui_pipeline->draw(
                item->element.uniform_data.descriptor_set, item->element.vertex_buffer, item->element.index_buffer);
        }
    }
    m_ui_pipeline->draw(m_select.uniform_data.descriptor_set, m_select.vertex_buffer, m_select.index_buffer);
}

void Hotbar::set_item(const int pos, const uint8_t block_type)
{
    auto [vertex_data, index_data] = create_item_mesh(block_type);
    Element element = { .uniform_data = m_ui_pipeline->create_uniform_data(),
                        .vertex_buffer = m_ui_pipeline->renderer().create_vertex_buffer(vertex_data),
                        .index_buffer = m_ui_pipeline->renderer().create_index_buffer(index_data) };
    element.uniform_data.descriptor_set.write_binding(m_texture_binding, m_atlas_texture);
    element.uniform_data.buffer.update(m_model_location, nnm::Matrix4f::identity());
    m_items[pos] = { block_type, std::move(element) };
}
