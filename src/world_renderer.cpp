#include "world_renderer.hpp"

#include "math/math.hpp"

void WorldRenderer::add_data(const ChunkData& chunk_data, const WorldData& world_data)
{
    ChunkMesh mesh(chunk_data.position(), world_data, *m_renderer);
    if (m_chunk_mesh_lookup.contains(chunk_data.position())) {
        m_chunk_meshes[m_chunk_mesh_lookup.at(chunk_data.position())] = std::move(mesh);
    }
    else {
        m_chunk_mesh_lookup[mesh.chunk_position()] = m_chunk_meshes.size();
        m_chunk_meshes.push_back(std::move(mesh));
    }
}

WorldRenderer::WorldRenderer(mve::Renderer& renderer)
    : m_renderer(&renderer)
    , m_vertex_shader(mve::Shader("../res/bin/shader/simple.vert.spv", mve::ShaderType::vertex))
    , m_fragment_shader(mve::Shader("../res/bin/shader/simple.frag.spv", mve::ShaderType::fragment))
    , m_graphics_pipeline(
          renderer.create_graphics_pipeline(m_vertex_shader, m_fragment_shader, chunk_vertex_layout(), true))
    , m_block_texture(std::make_shared<mve::Texture>(renderer, "../res/atlas.png"))
    , m_global_ubo(renderer.create_uniform_buffer(m_vertex_shader.descriptor_set(0).binding(0)))
    , m_global_descriptor_set(renderer.create_descriptor_set(m_graphics_pipeline, m_vertex_shader.descriptor_set(0)))
    , m_view_location(m_vertex_shader.descriptor_set(0).binding(0).member("view").location())
    , m_proj_location(m_vertex_shader.descriptor_set(0).binding(0).member("proj").location())
{
    m_global_descriptor_set.write_binding(m_vertex_shader.descriptor_set(0).binding(0), m_global_ubo);
    m_global_descriptor_set.write_binding(m_fragment_shader.descriptor_set(0).binding(1), *m_block_texture);
    m_frustum = {};
}

void WorldRenderer::resize()
{
    float angle = mve::radians(90.0f);
    float ratio = (float)(m_renderer->extent().x) / (float)(m_renderer->extent().y);
    float near = 0.01f;
    float far = 1000.0f;

    m_frustum.update_perspective(angle, ratio, near, far);
    mve::Matrix4 proj = mve::perspective(angle, ratio, near, far);
    m_global_ubo.update(m_proj_location, proj);
}
void WorldRenderer::set_view(const mve::Matrix4& view)
{
    m_global_ubo.update(m_view_location, view);
}
void WorldRenderer::draw(const Camera& camera)
{
    m_frustum.update_camera(camera);

    m_renderer->bind_graphics_pipeline(m_graphics_pipeline);

    for (const ChunkMesh& mesh : m_chunk_meshes) {
        if (m_frustum.contains_sphere(mesh.chunk_position() * 16.0f, 30.0f)) {
            m_renderer->bind_descriptor_set(m_global_descriptor_set);
            mesh.draw(*m_renderer);
        }
    }
}
void WorldRenderer::rebuild_mesh_lookup()
{
    m_chunk_mesh_lookup.clear();
    for (size_t i = 0; i < m_chunk_meshes.size(); i++) {
        m_chunk_mesh_lookup[m_chunk_meshes.at(i).chunk_position()] = i;
    }
}
