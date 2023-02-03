#include "chunk_mesh.hpp"

#include "math/functions.hpp"
#include "math/matrix4.hpp"

ChunkMesh::ChunkMesh(
    const ChunkData& data,
    mve::Renderer& renderer,
    mve::GraphicsPipeline& pipeline,
    mve::Shader& vertex_shader,
    mve::Shader& fragment_shader,
    std::shared_ptr<mve::Texture> texture)
    : m_descriptor_set(renderer.create_descriptor_set(pipeline, vertex_shader.descriptor_set(1)))
    , m_uniform_buffer(renderer.create_uniform_buffer(vertex_shader.descriptor_set(1).binding(0)))
    , m_mesh_buffers(create_buffers_from_chunk_data(renderer, data))
    , m_texture(texture)
    , m_model_location(vertex_shader.descriptor_set(1).binding(0).member("model").location())
{
    m_descriptor_set.write_binding(vertex_shader.descriptor_set(1).binding(0), m_uniform_buffer);
    m_descriptor_set.write_binding(fragment_shader.descriptor_set(1).binding(1), *m_texture.get());
    m_uniform_buffer.update(m_model_location, mve::Matrix4::identity());
}

void ChunkMesh::combine_mesh_data(MeshData& data, const MeshData& other)
{
    uint32_t indices_offset = data.vertices.size();
    for (int i = 0; i < other.vertices.size(); i++) {
        data.vertices.push_back(other.vertices[i]);
        data.uvs.push_back(other.uvs[i]);
    }

    for (int i = 0; i < other.indices.size(); i++) {
        data.indices.push_back(other.indices[i] + indices_offset);
    }
}

static const mve::VertexLayout standard_vertex_layout = {
    mve::VertexAttributeType::vec3, // Position
    mve::VertexAttributeType::vec3, // Color
    mve::VertexAttributeType::vec2 // UV
};

ChunkMesh::MeshBuffers ChunkMesh::create_buffers_from_chunk_data(mve::Renderer& renderer, const ChunkData& chunk_data)
{
    MeshData mesh;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                for (int f = 0; f < 6; f++) {
                    MeshData face_mesh = create_face_mesh(mve::Vector3(x, y, z), static_cast<BlockFace>(f));
                    combine_mesh_data(mesh, face_mesh);
                }
            }
        }
    }

    mve::VertexData quad_vertex_data(standard_vertex_layout);
    for (int i = 0; i < mesh.vertices.size(); i++) {
        quad_vertex_data.push_back(mesh.vertices.at(i));
        quad_vertex_data.push_back({ 1, 1, 1 });
        quad_vertex_data.push_back(mesh.uvs.at(i));
    }

    return { renderer.create_vertex_buffer(quad_vertex_data), renderer.create_index_buffer(mesh.indices) };
}
void ChunkMesh::draw(mve::Renderer& renderer, mve::DescriptorSet& global_descriptor_set)
{
    renderer.bind_descriptor_sets({ global_descriptor_set, m_descriptor_set });
    renderer.bind_vertex_buffer(m_mesh_buffers.vertex_buffer);
    renderer.draw_index_buffer(m_mesh_buffers.index_buffer);
}

ChunkMesh::MeshData ChunkMesh::create_face_mesh(mve::Vector3 offset, BlockFace face)
{
    MeshData data;
    switch (face) {
    case BlockFace::front:
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back({ 0.0f, 0.0f });
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0 });
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0.5f });
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0, 0.5f });
        data.indices = { 0, 2, 3, 0, 1, 2 };
        return data;
    case BlockFace::back:
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back({ 0.0f, 0.0f });
        data.vertices.push_back(mve::Vector3(-0.5f, 0 - .5f, 0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0 });
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0.5f });
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0, 0.5f });
        data.indices = { 0, 2, 3, 0, 1, 2 };
        return data;
    case BlockFace::left:
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back({ 0.0f, 0.0f });
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0 });
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0.5f });
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0, 0.5f });
        data.indices = { 0, 2, 3, 0, 1, 2 };
        return data;
    case BlockFace::right:
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back({ 0.0f, 0.0f });
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0 });
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0.5f });
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0, 0.5f });
        data.indices = { 0, 2, 3, 0, 1, 2 };
        return data;
    case BlockFace::top:
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0.0f });
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back({ 1.0f, 0 });
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back({ 1.0f, 0.5f });
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0.5f });
        data.indices = { 0, 2, 3, 0, 1, 2 };
        return data;
    case BlockFace::bottom:
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0.0f, 0.5f });
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0.5f, 0.5f });
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0.5f, 1.0f });
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back({ 0.0f, 1.0f });
        data.indices = { 0, 2, 3, 0, 1, 2 };
        return data;
    default:
        return data;
    }
}

ChunkMesh::FaceUVs ChunkMesh::uvs_from_atlas(
    int tex_width, int tex_height, int atlas_width, int atlas_height, int x, int y)
{
    return ChunkMesh::FaceUVs();
}
