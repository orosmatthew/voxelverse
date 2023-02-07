#include "world_renderer.hpp"

#include "math/math.hpp"

void WorldRenderer::add_data(const ChunkData& chunk_data, const WorldData& world_data)
{
    if (m_chunk_meshes.contains(chunk_data.position())) {
        m_chunk_meshes.erase(chunk_data.position());
    }
    ChunkMesh mesh(chunk_data.position(), world_data, *m_renderer);
    m_chunk_meshes.insert({ chunk_data.position(), std::move(mesh) });
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
}

void WorldRenderer::resize()
{
    mve::Matrix4 proj = mve::perspective(
        mve::radians(90.0f), (float)(m_renderer->extent().x) / (float)(m_renderer->extent().y), 0.01f, 1000.0f);
    m_global_ubo.update(m_proj_location, proj);
}
void WorldRenderer::set_view(const mve::Matrix4& view)
{
    m_global_ubo.update(m_view_location, view);
}
void WorldRenderer::draw()
{
    m_renderer->bind_graphics_pipeline(m_graphics_pipeline);

    for (auto& [pos, mesh] : m_chunk_meshes) {
        m_renderer->bind_descriptor_set(m_global_descriptor_set);
        mesh.draw(*m_renderer);
    }
}
mve::VertexLayout WorldRenderer::chunk_vertex_layout()
{
    return {
        mve::VertexAttributeType::vec3, // Position
        mve::VertexAttributeType::vec3, // Color
        mve::VertexAttributeType::vec2 // UV
    };
}
