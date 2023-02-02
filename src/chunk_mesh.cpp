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

ChunkMesh::MeshData ChunkMesh::create_quad_mesh(
    mve::Vector3 pos_top_left,
    mve::Vector3 pos_top_right,
    mve::Vector3 pos_bottom_right,
    mve::Vector3 pos_bottom_left,
    mve::Vector2 uv_top_left,
    mve::Vector2 uv_top_right,
    mve::Vector2 uv_bottom_right,
    mve::Vector2 uv_bottom_left)
{
    MeshData mesh_data;
    mesh_data.vertices.push_back(pos_top_left);
    mesh_data.uvs.push_back(uv_top_left);

    mesh_data.vertices.push_back(pos_top_right);
    mesh_data.uvs.push_back(uv_top_right);

    mesh_data.vertices.push_back(pos_bottom_right);
    mesh_data.uvs.push_back(uv_bottom_right);

    mesh_data.vertices.push_back(pos_bottom_left);
    mesh_data.uvs.push_back(uv_bottom_left);

    mesh_data.indices = { 0, 2, 3, 0, 1, 2 };

    return mesh_data;
}

void ChunkMesh::push_data(MeshData& data, const MeshData& face)
{
    uint32_t indices_offset = data.vertices.size();
    for (int i = 0; i < face.vertices.size(); i++) {
        data.vertices.push_back(face.vertices[i]);
        data.uvs.push_back(face.uvs[i]);
    }

    for (int i = 0; i < face.indices.size(); i++) {
        data.indices.push_back(face.indices[i] + indices_offset);
    }
}

ChunkMesh::MeshData ChunkMesh::create_cube_mesh()
{
    MeshData data;

    std::array<mve::Vector3, 4> quad_verts { mve::Vector3(-0.5f, 0.5f, 0.5f),
                                             mve::Vector3(0.5f, 0.5f, 0.5f),
                                             mve::Vector3(0.5f, 0.5f, -0.5f),
                                             mve::Vector3(-0.5f, 0.5f, -0.5f) };

    MeshData front_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0, 0 }, { 0.5f, 0 }, { 0.5f, 0.5f }, { 0, 0.5f });
    push_data(data, front_face_data);

    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(0, 0, 1), mve::pi / 2.0f);
    }

    MeshData right_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0, 0 }, { 0.5f, 0 }, { 0.5f, 0.5f }, { 0, 0.5f });
    push_data(data, right_face_data);

    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(0, 0, 1), mve::pi / 2.0f);
    }

    MeshData back_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0, 0 }, { 0.5f, 0 }, { 0.5f, 0.5f }, { 0, 0.5f });
    push_data(data, back_face_data);

    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(0, 0, 1), mve::pi / 2.0f);
    }

    MeshData left_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0, 0 }, { 0.5f, 0 }, { 0.5f, 0.5f }, { 0, 0.5f });
    push_data(data, left_face_data);

    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(0, 0, 1), mve::pi / 2.0f);
    }
    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(1, 0, 0), mve::pi / 2.0f);
    }

    MeshData top_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0.5f, 0 }, { 1, 0 }, { 1, 0.5f }, { 0.5f, 0.5f });
    push_data(data, top_face_data);

    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(1, 0, 0), mve::pi);
    }

    MeshData bottom_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0, 0.5f }, { 0.5f, 0.5f }, { 0.5f, 1 }, { 0, 1 });
    push_data(data, bottom_face_data);

    return data;
}

static const mve::VertexLayout standard_vertex_layout = {
    mve::VertexAttributeType::vec3, // Position
    mve::VertexAttributeType::vec3, // Color
    mve::VertexAttributeType::vec2 // UV
};

ChunkMesh::MeshBuffers ChunkMesh::create_buffers_from_chunk_data(mve::Renderer& renderer, const ChunkData& chunk_data)
{
    MeshData quad_mesh = create_cube_mesh();
    //= create_quad_mesh({ -1, 0, 1 }, { 1, 0, 1 }, { -1, 0, -1 }, { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 });

    mve::VertexData quad_vertex_data(standard_vertex_layout);
    for (int i = 0; i < quad_mesh.vertices.size(); i++) {
        quad_vertex_data.push_back(quad_mesh.vertices.at(i));
        quad_vertex_data.push_back({ 1, 1, 1 });
        quad_vertex_data.push_back(quad_mesh.uvs.at(i));
    }

    return { renderer.create_vertex_buffer(quad_vertex_data), renderer.create_index_buffer(quad_mesh.indices) };
}
void ChunkMesh::draw(mve::Renderer& renderer, mve::DescriptorSet& global_descriptor_set)
{
    renderer.bind_descriptor_sets({ global_descriptor_set, m_descriptor_set });
    renderer.bind_vertex_buffer(m_mesh_buffers.vertex_buffer);
    renderer.draw_index_buffer(m_mesh_buffers.index_buffer);
}