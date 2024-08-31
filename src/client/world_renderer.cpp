#include "world_renderer.hpp"

#include "common.hpp"

#include <nnm/nnm.hpp>

void WorldRenderer::push_mesh_update(nnm::Vector3i chunk_pos)
{
    if (m_chunk_mesh_lookup.contains(chunk_pos)) {
        m_chunk_buffers.at(m_chunk_mesh_lookup.at(chunk_pos)).reset();
    }
    else {
        m_chunk_mesh_lookup.insert({ chunk_pos, m_chunk_buffers.size() });
        m_chunk_buffers.emplace_back();
    }
    m_chunk_mesh_update_list.push_back(chunk_pos);
}

WorldRenderer::WorldRenderer(mve::Renderer& renderer)
    : m_renderer(&renderer)
    , m_vertex_shader(mve::Shader(res_path("bin/shader/simple.vert.spv")))
    , m_fragment_shader(mve::Shader(res_path("bin/shader/simple.frag.spv")))
    , m_graphics_pipeline(renderer.create_graphics_pipeline(m_vertex_shader, m_fragment_shader, vertex_layout(), true))
    , m_block_texture(std::make_shared<mve::Texture>(renderer, res_path("atlas.png")))
    , m_global_ubo(renderer.create_uniform_buffer(m_vertex_shader.descriptor_set(0).binding(0)))
    , m_chunk_ubo(renderer.create_uniform_buffer(m_vertex_shader.descriptor_set(1).binding(0)))
    , m_global_descriptor_set(renderer.create_descriptor_set(m_graphics_pipeline, m_vertex_shader.descriptor_set(0)))
    , m_chunk_descriptor_set(m_graphics_pipeline.create_descriptor_set(m_vertex_shader.descriptor_set(1)))
    , m_view_location(m_vertex_shader.descriptor_set(0).binding(0).member("view").location())
    , m_proj_location(m_vertex_shader.descriptor_set(0).binding(0).member("proj").location())
    , m_selection_box(SelectionBox {
          .is_shown = true,
          .mesh = WireBoxMesh(
              renderer,
              m_graphics_pipeline,
              m_vertex_shader.descriptor_set(1),
              m_vertex_shader.descriptor_set(1).binding(0),
              BoundingBox { .min = { -0.5f, -0.5f, -0.5f }, .max = { 0.5f, 0.5f, 0.5f } },
              0.01f,
              { 0.0f, 0.0f, 0.0f }) })
{
    m_global_descriptor_set.write_binding(m_vertex_shader.descriptor_set(0).binding(0), m_global_ubo);
    m_global_descriptor_set.write_binding(m_fragment_shader.descriptor_set(0).binding(1), *m_block_texture);
    m_chunk_ubo.update(
        m_vertex_shader.descriptor_set(1).binding(0).member("model").location(), nnm::Matrix4f::identity());
    m_chunk_ubo.update(m_vertex_shader.descriptor_set(1).binding(0).member("fog_influence").location(), 1.0f);
    m_chunk_descriptor_set.write_binding(m_vertex_shader.descriptor_set(1).binding(0), m_chunk_ubo);
    m_frustum = {};
    m_selection_box.mesh.set_position({ 0, 0, 0 });

    m_global_ubo.update(
        m_vertex_shader.descriptor_set(0).binding(0).member("fog_color").location(),
        nnm::Vector4(142.0f / 255.0f, 186.0f / 255.0f, 1.0f, 1.0f));
    m_global_ubo.update(m_vertex_shader.descriptor_set(0).binding(0).member("fog_near").location(), 400.0f);
    m_global_ubo.update(m_vertex_shader.descriptor_set(0).binding(0).member("fog_far").location(), 475.0f);
}

void WorldRenderer::resize()
{
    constexpr float angle = nnm::radians(90.0f);
    const float ratio = static_cast<float>(m_renderer->extent().x) / static_cast<float>(m_renderer->extent().y);
    constexpr float near = 0.01f;
    constexpr float far = 10000.0f;

    m_frustum.update_perspective(angle, ratio, near, far);
    const auto proj = nnm::Transform3f::from_projection_perspective_right_hand_0to1(angle, ratio, near, far)
                          .rotate_axis_angle_local(nnm::Vector3f::axis_x(), nnm::pi() / 2.0f)
                          .scale({ 1, -1, 1 });
    m_global_ubo.update(m_proj_location, proj.matrix);
}
void WorldRenderer::set_view(const nnm::Matrix4f& view)
{
    m_global_ubo.update(m_view_location, view);
}
void WorldRenderer::draw(const Player& camera)
{
    m_frustum.update_camera(camera);

    m_renderer->bind_graphics_pipeline(m_graphics_pipeline);

    if (m_selection_box.is_shown) {
        m_selection_box.mesh.draw(m_global_descriptor_set);
    }

    for (const std::optional<ChunkBuffers>& mesh : m_chunk_buffers) {
        // TODO: Fix frustum culling
        // if (mesh.has_value() && m_frustum.contains_sphere(nnm::Vector3f(mesh->chunk_pos()) * 16.0f, 30.0f)) {
        if (mesh.has_value()) {
            m_renderer->bind_descriptor_sets(m_global_descriptor_set, m_chunk_descriptor_set);
            mesh->draw(*m_renderer);
        }
    }

    for (const auto& [id, box] : m_debug_boxes) {
        if (box.is_shown) {
            box.mesh.draw(m_global_descriptor_set);
        }
    }
}
// void WorldRenderer::rebuild_mesh_lookup()
// {
//     m_chunk_mesh_lookup.clear();
//     for (size_t i = 0; i < m_chunk_buffers.size(); i++) {
//         if (m_chunk_buffers[i].has_value()) {
//             m_chunk_mesh_lookup[m_chunk_buffers.at(i)->chunk_pos()] = i;
//         }
//     }
// }
void WorldRenderer::set_selection_position(const nnm::Vector3f position)
{
    m_selection_box.mesh.set_position(position);
}
bool WorldRenderer::contains_data(const nnm::Vector3i position) const
{
    return m_chunk_mesh_lookup.contains(position);
}
void WorldRenderer::remove_data(const nnm::Vector3i position)
{
    m_chunk_buffers.at(m_chunk_mesh_lookup.at(position)).reset();
    m_chunk_mesh_lookup.erase(position);
}

uint64_t WorldRenderer::create_debug_box(const BoundingBox& box, const float width, const nnm::Vector3f color)
{
    WireBoxMesh box_mesh(
        *m_renderer,
        m_graphics_pipeline,
        m_vertex_shader.descriptor_set(1),
        m_vertex_shader.descriptor_set(1).binding(0),
        box,
        width,
        color);
    DebugBox debug_box { .is_shown = true, .mesh = std::move(box_mesh) };
    uint64_t count = 0;
    while (true) {
        if (!m_debug_boxes.contains(count)) {
            m_debug_boxes.insert({ count, std::move(debug_box) });
            break;
        }
        count++;
    }
    return count;
}

void WorldRenderer::hide_debug_box(const uint64_t id)
{
    m_debug_boxes.at(id).is_shown = false;
}
void WorldRenderer::show_debug_box(const uint64_t id)
{
    m_debug_boxes.at(id).is_shown = true;
}
void WorldRenderer::delete_debug_box(const uint64_t id)
{
    m_debug_boxes.erase(id);
}
void WorldRenderer::delete_all_debug_boxes()
{
    m_debug_boxes.clear();
}
void WorldRenderer::process_mesh_updates(const WorldData& world_data)
{
    std::erase_if(m_chunk_mesh_update_list, [&](const nnm::Vector3i& chunk_pos) {
        return !m_chunk_mesh_lookup.contains(chunk_pos);
    });
    m_temp_chunk_buffer_data.clear();
    m_temp_chunk_buffer_data.resize(m_chunk_mesh_update_list.size());
    const BS::multi_future<void> tasks = m_thread_pool.submit_blocks<size_t>(
        0, m_chunk_mesh_update_list.size(), [&](const auto begin, const auto end) {
            for (auto i = begin; i < end; ++i) {
                const nnm::Vector3i chunk_pos = m_chunk_mesh_update_list[i];
                m_temp_chunk_buffer_data[i] = std::move(create_chunk_buffer_data(chunk_pos, world_data));
            }
        });
    tasks.wait();
    for (const std::optional<ChunkBufferData>& buffer_data : m_temp_chunk_buffer_data) {
        if (buffer_data.has_value()) {
            ChunkBuffers buffers(*m_renderer, buffer_data.value());
            m_chunk_buffers[m_chunk_mesh_lookup[buffers.chunk_pos()]] = std::move(buffers);
        }
    }
    m_chunk_mesh_update_list.clear();
}
