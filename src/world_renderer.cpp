#include "world_renderer.hpp"
#include "mve/math/math.hpp"

void WorldRenderer::queue_update(mve::Vector3i chunk_pos)
{
    ChunkMesh mesh;
    if (m_chunk_mesh_lookup.contains(chunk_pos)) {
        m_chunk_meshes[m_chunk_mesh_lookup.at(chunk_pos)] = std::move(mesh);
    }
    else {
        m_chunk_mesh_lookup.insert({ chunk_pos, m_chunk_meshes.size() });
        m_chunk_meshes.push_back(std::move(mesh));
    }
    m_chunk_update_queue.push_back(chunk_pos);
}

WorldRenderer::WorldRenderer(mve::Renderer& renderer)
    : m_renderer(&renderer)
    , m_thread_pool()
    , m_vertex_shader(mve::Shader("../res/bin/shader/simple.vert.spv"))
    , m_fragment_shader(mve::Shader("../res/bin/shader/simple.frag.spv"))
    , m_graphics_pipeline(renderer.create_graphics_pipeline(m_vertex_shader, m_fragment_shader, vertex_layout(), true))
    , m_block_texture(std::make_shared<mve::Texture>(renderer, "../res/atlas.png"))
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
        m_vertex_shader.descriptor_set(1).binding(0).member("model").location(), mve::Matrix4::identity());
    m_chunk_ubo.update(m_vertex_shader.descriptor_set(1).binding(0).member("fog_influence").location(), 1.0f);
    m_chunk_descriptor_set.write_binding(m_vertex_shader.descriptor_set(1).binding(0), m_chunk_ubo);
    m_frustum = {};
    m_selection_box.mesh.set_position({ 0, 0, 0 });

    m_global_ubo.update(
        m_vertex_shader.descriptor_set(0).binding(0).member("fog_color").location(),
        mve::Vector4(142.0f / 255.0f, 186.0f / 255.0f, 255.0f / 255.0f, 1.0f));
    m_global_ubo.update(m_vertex_shader.descriptor_set(0).binding(0).member("fog_near").location(), 400.0f);
    m_global_ubo.update(m_vertex_shader.descriptor_set(0).binding(0).member("fog_far").location(), 475.0f);
}

void WorldRenderer::resize()
{
    float angle = mve::radians(90.0f);
    float ratio = (float)(m_renderer->extent().x) / (float)(m_renderer->extent().y);
    float near = 0.01f;
    float far = 10000.0f;

    m_frustum.update_perspective(angle, ratio, near, far);
    mve::Matrix4 proj = mve::perspective(angle, ratio, near, far);
    m_global_ubo.update(m_proj_location, proj);
}
void WorldRenderer::set_view(const mve::Matrix4& view)
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

    for (const std::optional<ChunkMesh>& mesh : m_chunk_meshes) {
        if (mesh.has_value() && m_frustum.contains_sphere(mesh->chunk_position() * 16.0f, 30.0f)) {
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
void WorldRenderer::rebuild_mesh_lookup()
{
    m_chunk_mesh_lookup.clear();
    for (size_t i = 0; i < m_chunk_meshes.size(); i++) {
        if (m_chunk_meshes[i].has_value()) {
            m_chunk_mesh_lookup[m_chunk_meshes.at(i)->chunk_position()] = i;
        }
    }
}
void WorldRenderer::set_selection_position(mve::Vector3 position)
{
    m_selection_box.mesh.set_position(position);
}
bool WorldRenderer::contains_data(mve::Vector3i position)
{
    return m_chunk_mesh_lookup.contains(position);
}
void WorldRenderer::remove_data(mve::Vector3i position)
{
    m_chunk_meshes[m_chunk_mesh_lookup.at(position)].reset();
    m_chunk_mesh_lookup.erase(position);
}

uint64_t WorldRenderer::create_debug_box(const BoundingBox& box, float width, mve::Vector3 color)
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

void WorldRenderer::hide_debug_box(uint64_t id)
{
    m_debug_boxes.at(id).is_shown = false;
}
void WorldRenderer::show_debug_box(uint64_t id)
{
    m_debug_boxes.at(id).is_shown = true;
}
void WorldRenderer::delete_debug_box(uint64_t id)
{
    m_debug_boxes.erase(id);
}
void WorldRenderer::delete_all_debug_boxes()
{
    m_debug_boxes.clear();
}
void WorldRenderer::process_updates(const WorldData& world_data)
{
    std::erase_if(m_chunk_update_queue, [&](const mve::Vector3i& chunk_pos) {
        return !m_chunk_mesh_lookup.contains(chunk_pos);
    });
    m_thread_pool
        .parallelize_loop(
            m_chunk_update_queue.size(),
            [&](const size_t& a, const size_t& b) {
                for (size_t i = a; i < b; i++) {
                    ChunkMesh& mesh = *m_chunk_meshes[m_chunk_mesh_lookup[m_chunk_update_queue[i]]];
                    mesh.create_mesh_data(m_chunk_update_queue[i], world_data);
                }
            })
        .wait();
    m_thread_pool.wait_for_tasks();
    for (const mve::Vector3i& chunk_pos : m_chunk_update_queue) {
        m_chunk_meshes[m_chunk_mesh_lookup[chunk_pos]]->create_buffers(*m_renderer);
    }
    m_chunk_update_queue.clear();
}
