#include "chunk_mesh.hpp"

#include "math/matrix4.hpp"
#include "math/vector2i.hpp"

ChunkMesh::ChunkMesh(
    const ChunkData& data,
    mve::Renderer& renderer,
    mve::GraphicsPipeline& pipeline,
    mve::Shader& vertex_shader,
    mve::Shader& fragment_shader,
    std::shared_ptr<mve::Texture> texture)
    : m_transform(mve::Matrix4::from_basis_translation(mve::Matrix3::identity(), data.position() * 16.0f))
    , m_descriptor_set(renderer.create_descriptor_set(pipeline, vertex_shader.descriptor_set(1)))
    , m_uniform_buffer(renderer.create_uniform_buffer(vertex_shader.descriptor_set(1).binding(0)))
    , m_mesh_buffers(create_buffers_from_chunk_data(renderer, data))
    , m_texture(texture)
    , m_model_location(vertex_shader.descriptor_set(1).binding(0).member("model").location())
{
    m_descriptor_set.write_binding(vertex_shader.descriptor_set(1).binding(0), m_uniform_buffer);
    m_descriptor_set.write_binding(fragment_shader.descriptor_set(1).binding(1), *m_texture.get());
    m_uniform_buffer.update(m_model_location, m_transform);
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

std::optional<ChunkMesh::MeshBuffers> ChunkMesh::create_buffers_from_chunk_data(
    mve::Renderer& renderer, const ChunkData& chunk_data)
{
    bool empty = true;
    MeshData mesh;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                if (chunk_data.get_block(mve::Vector3i(x, y, z)) == 0) {
                    continue;
                }
                empty = false;
                for (int f = 0; f < 6; f++) {
                    Direction dir = static_cast<Direction>(f);
                    mve::Vector3i adj_block = mve::Vector3i(x, y, z) + direction_vector(dir);
                    if (chunk_data.in_bounds(adj_block)) {
                        if (chunk_data.get_block(adj_block) == 0) {
                            MeshData face_mesh = create_face_mesh(mve::Vector3(x, y, z), dir);
                            combine_mesh_data(mesh, face_mesh);
                        }
                        else {
                            continue;
                        }
                    }
                    else {
                        MeshData face_mesh = create_face_mesh(mve::Vector3(x, y, z), dir);
                        combine_mesh_data(mesh, face_mesh);
                    }
                }
            }
        }
    }
    if (empty) {
        return {};
    }

    mve::VertexData quad_vertex_data(standard_vertex_layout);
    for (int i = 0; i < mesh.vertices.size(); i++) {
        quad_vertex_data.push_back(mesh.vertices.at(i));
        quad_vertex_data.push_back({ 1, 1, 1 });
        quad_vertex_data.push_back(mesh.uvs.at(i));
    }

    return MeshBuffers { renderer.create_vertex_buffer(quad_vertex_data), renderer.create_index_buffer(mesh.indices) };
}
void ChunkMesh::draw(mve::Renderer& renderer, mve::DescriptorSet& global_descriptor_set)
{
    if (m_mesh_buffers.has_value()) {
        renderer.bind_descriptor_sets({ global_descriptor_set, m_descriptor_set });
        renderer.bind_vertex_buffer(m_mesh_buffers.value().vertex_buffer);
        renderer.draw_index_buffer(m_mesh_buffers.value().index_buffer);
    }
}

ChunkMesh::MeshData ChunkMesh::create_face_mesh(mve::Vector3 offset, Direction face)
{
    MeshData data;
    FaceUVs uvs;
    switch (face) {
    case Direction::front:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 0, 0 });
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.top_left);
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.top_right);
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.bottom_right);
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.bottom_left);
        data.indices = { 0, 3, 2, 0, 2, 1 };
        return data;
    case Direction::back:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 0, 0 });
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.top_left);
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.top_right);
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.bottom_right);
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.bottom_left);
        data.indices = { 0, 3, 2, 0, 2, 1 };
        return data;
    case Direction::left:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 0, 0 });
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.top_left);
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.top_right);
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.bottom_right);
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.bottom_left);
        data.indices = { 0, 3, 2, 0, 2, 1 };
        return data;
    case Direction::right:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 0, 0 });
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.top_left);
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.top_right);
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.bottom_right);
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.bottom_left);
        data.indices = { 0, 3, 2, 0, 2, 1 };
        return data;
    case Direction::top:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 1, 0 });
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.top_left);
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.top_right);
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.bottom_right);
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, 0.5f) + offset);
        data.uvs.push_back(uvs.bottom_left);
        data.indices = { 0, 3, 2, 0, 2, 1 };
        return data;
    case Direction::bottom:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 0, 1 });
        data.vertices.push_back(mve::Vector3(0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.top_left);
        data.vertices.push_back(mve::Vector3(-0.5f, 0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.top_right);
        data.vertices.push_back(mve::Vector3(-0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.bottom_right);
        data.vertices.push_back(mve::Vector3(0.5f, -0.5f, -0.5f) + offset);
        data.uvs.push_back(uvs.bottom_left);
        data.indices = { 0, 3, 2, 0, 2, 1 };
        return data;
    default:
        return data;
    }
}

ChunkMesh::FaceUVs ChunkMesh::uvs_from_atlas(mve::Vector2i texture_size, mve::Vector2i atlas_size, mve::Vector2i pos)
{
    mve::Vector2 atlas_unit = mve::Vector2(1.0f / atlas_size.x, 1.0f / atlas_size.y);
    mve::Vector2 padding = atlas_unit / (mve::Vector2(texture_size) / mve::Vector2(atlas_size));

    FaceUVs uvs;
    uvs.top_left = mve::Vector2(pos.x * atlas_unit.x, pos.y * atlas_unit.y);
    uvs.top_right = uvs.top_left + mve::Vector2(atlas_unit.x, 0.0f);
    uvs.bottom_right = uvs.top_right + mve::Vector2(0.0f, atlas_unit.y);
    uvs.bottom_left = uvs.bottom_right + mve::Vector2(-atlas_unit.x, 0.0f);

    uvs.top_left += padding;
    uvs.top_right += mve::Vector2(-padding.x, padding.y);
    uvs.bottom_right += mve::Vector2(-padding.x, -padding.y);
    uvs.bottom_left += mve::Vector2(padding.x, -padding.y);

    return uvs;
}
