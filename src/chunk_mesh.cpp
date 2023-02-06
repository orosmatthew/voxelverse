#include "chunk_mesh.hpp"

#include "math/math.hpp"
#include "world_renderer.hpp"

ChunkMesh::ChunkMesh(mve::Vector3i chunk_pos, const WorldData& data, mve::Renderer& renderer)
    : m_mesh_buffers(create_buffers(chunk_pos, renderer, data))
{
}

void ChunkMesh::combine_mesh_data(MeshData& data, const MeshData& other)
{
    uint32_t indices_offset = data.vertices.size();
    for (int i = 0; i < other.vertices.size(); i++) {
        data.vertices.push_back(other.vertices[i]);
        data.colors.push_back(other.colors[i]);
        data.uvs.push_back(other.uvs[i]);
    }

    for (int i = 0; i < other.indices.size(); i++) {
        data.indices.push_back(other.indices[i] + indices_offset);
    }
}

std::optional<ChunkMesh::MeshBuffers> ChunkMesh::create_buffers(
    mve::Vector3i chunk_pos, mve::Renderer& renderer, const WorldData& world_data)
{
    bool empty = true;
    MeshData mesh;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                mve::Vector3i local_pos { x, y, z };
                std::optional<uint8_t> block = world_data.block_at_relative(chunk_pos, local_pos);
                if (block.has_value() && block.value() == 0) {
                    continue;
                }
                empty = false;
                for (int f = 0; f < 6; f++) {
                    Direction dir = static_cast<Direction>(f);
                    mve::Vector3i adj_local_pos = local_pos + direction_vector(dir);
                    std::optional<uint8_t> adj_block = world_data.block_at_relative(chunk_pos, adj_local_pos);
                    if (adj_block.has_value()) {
                        if (adj_block.value() == 0) {
                            std::array<uint8_t, 4> face_lighting
                                = calc_face_lighting(world_data, chunk_pos, local_pos, dir);
                            FaceData face = create_face_mesh(mve::Vector3(x, y, z), dir, face_lighting);
                            add_face_to_mesh(mesh, face);
                        }
                        else {
                            continue;
                        }
                    }
                    else {
                        std::array<uint8_t, 4> face_lighting
                            = calc_face_lighting(world_data, chunk_pos, local_pos, dir);
                        FaceData face = create_face_mesh(mve::Vector3(x, y, z), dir, face_lighting);
                        add_face_to_mesh(mesh, face);
                    }
                }
            }
        }
    }
    if (empty) {
        return {};
    }

    mve::VertexData vertex_data(WorldRenderer::chunk_vertex_layout());
    for (int i = 0; i < mesh.vertices.size(); i++) {
        vertex_data.push_back(mesh.vertices.at(i) + chunk_pos * 16.0f);
        vertex_data.push_back(mesh.colors.at(i));
        vertex_data.push_back(mesh.uvs.at(i));
    }

    return MeshBuffers { renderer.create_vertex_buffer(vertex_data), renderer.create_index_buffer(mesh.indices) };
}
void ChunkMesh::draw(mve::Renderer& renderer)
{
    if (m_mesh_buffers.has_value()) {
        renderer.bind_vertex_buffer(m_mesh_buffers.value().vertex_buffer);
        renderer.draw_index_buffer(m_mesh_buffers.value().index_buffer);
    }
}

ChunkMesh::FaceData ChunkMesh::create_face_mesh(
    mve::Vector3 offset, Direction face, const std::array<uint8_t, 4>& lighting)
{
    FaceData data;
    FaceUVs uvs;
    switch (face) {
    case Direction::front:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 0, 0 });
        data.vertices[0] = mve::Vector3(-0.5f, -0.5f, 0.5f) + offset;
        data.vertices[1] = mve::Vector3(0.5f, -0.5f, 0.5f) + offset;
        data.vertices[2] = mve::Vector3(0.5f, -0.5f, -0.5f) + offset;
        data.vertices[3] = mve::Vector3(-0.5f, -0.5f, -0.5f) + offset;
        break;
    case Direction::back:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 0, 0 });
        data.vertices[0] = mve::Vector3(0.5f, 0.5f, 0.5f) + offset;
        data.vertices[1] = mve::Vector3(-0.5f, 0.5f, 0.5f) + offset;
        data.vertices[2] = mve::Vector3(-0.5f, 0.5f, -0.5f) + offset;
        data.vertices[3] = mve::Vector3(0.5f, 0.5f, -0.5f) + offset;
        break;
    case Direction::left:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 0, 0 });
        data.vertices[0] = mve::Vector3(-0.5f, 0.5f, 0.5f) + offset;
        data.vertices[1] = mve::Vector3(-0.5f, -0.5f, 0.5f) + offset;
        data.vertices[2] = mve::Vector3(-0.5f, -0.5f, -0.5f) + offset;
        data.vertices[3] = mve::Vector3(-0.5f, 0.5f, -0.5f) + offset;
        break;
    case Direction::right:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 0, 0 });
        data.vertices[0] = mve::Vector3(0.5f, -0.5f, 0.5f) + offset;
        data.vertices[1] = mve::Vector3(0.5f, 0.5f, 0.5f) + offset;
        data.vertices[2] = mve::Vector3(0.5f, 0.5f, -0.5f) + offset;
        data.vertices[3] = mve::Vector3(0.5f, -0.5f, -0.5f) + offset;
        break;
    case Direction::top:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 1, 0 });
        data.vertices[0] = mve::Vector3(-0.5f, 0.5f, 0.5f) + offset;
        data.vertices[1] = mve::Vector3(0.5f, 0.5f, 0.5f) + offset;
        data.vertices[2] = mve::Vector3(0.5f, -0.5f, 0.5f) + offset;
        data.vertices[3] = mve::Vector3(-0.5f, -0.5f, 0.5f) + offset;
        break;
    case Direction::bottom:
        uvs = uvs_from_atlas({ 32, 32 }, { 2, 2 }, { 0, 1 });
        data.vertices[0] = mve::Vector3(0.5f, 0.5f, -0.5f) + offset;
        data.vertices[1] = mve::Vector3(-0.5f, 0.5f, -0.5f) + offset;
        data.vertices[2] = mve::Vector3(-0.5f, -0.5f, -0.5f) + offset;
        data.vertices[3] = mve::Vector3(0.5f, -0.5f, -0.5f) + offset;
        break;
    default:
        break;
    }
    data.uvs[0] = uvs.top_left;
    data.uvs[1] = uvs.top_right;
    data.uvs[2] = uvs.bottom_right;
    data.uvs[3] = uvs.bottom_left;
    data.colors[0] = { lighting[0] / 255.0f, lighting[0] / 255.0f, lighting[0] / 255.0f };
    data.colors[1] = { lighting[1] / 255.0f, lighting[1] / 255.0f, lighting[1] / 255.0f };
    data.colors[2] = { lighting[2] / 255.0f, lighting[2] / 255.0f, lighting[2] / 255.0f };
    data.colors[3] = { lighting[3] / 255.0f, lighting[3] / 255.0f, lighting[3] / 255.0f };
    data.indices = { 0, 3, 2, 0, 2, 1 };
    return data;
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

void ChunkMesh::add_face_to_mesh(ChunkMesh::MeshData& data, const ChunkMesh::FaceData& face)
{
    uint32_t indices_offset = data.vertices.size();
    for (int i = 0; i < face.vertices.size(); i++) {
        data.vertices.push_back(face.vertices[i]);
        data.colors.push_back(face.colors[i]);
        data.uvs.push_back(face.uvs[i]);
    }

    for (int i = 0; i < face.indices.size(); i++) {
        data.indices.push_back(face.indices[i] + indices_offset);
    }
}

std::array<uint8_t, 4> ChunkMesh::calc_face_lighting(
    const WorldData& data, mve::Vector3i chunk_pos, mve::Vector3i local_block_pos, Direction dir)
{
    std::array<mve::Vector3i, 8> check_blocks;
    switch (dir) {
    case Direction::front:
        check_blocks[0] = { -1, -1, 1 };
        check_blocks[1] = { 0, -1, 1 };
        check_blocks[2] = { 1, -1, 1 };
        check_blocks[3] = { 1, -1, 0 };
        check_blocks[4] = { 1, -1, -1 };
        check_blocks[5] = { 0, -1, -1 };
        check_blocks[6] = { -1, -1, -1 };
        check_blocks[7] = { -1, -1, 0 };
        break;
    case Direction::back:
        check_blocks[0] = { 1, 1, 1 };
        check_blocks[1] = { 0, 1, 1 };
        check_blocks[2] = { -1, 1, 1 };
        check_blocks[3] = { -1, 1, 0 };
        check_blocks[4] = { -1, 1, -1 };
        check_blocks[5] = { 0, 1, -1 };
        check_blocks[6] = { 1, 1, -1 };
        check_blocks[7] = { 1, 1, 0 };
        break;
    case Direction::left:
        check_blocks[0] = { -1, 1, 1 };
        check_blocks[1] = { -1, 0, 1 };
        check_blocks[2] = { -1, -1, 1 };
        check_blocks[3] = { -1, -1, 0 };
        check_blocks[4] = { -1, -1, -1 };
        check_blocks[5] = { -1, 0, -1 };
        check_blocks[6] = { -1, 1, -1 };
        check_blocks[7] = { -1, 1, 0 };
        break;
    case Direction::right:
        check_blocks[0] = { 1, -1, 1 };
        check_blocks[1] = { 1, 0, 1 };
        check_blocks[2] = { 1, 1, 1 };
        check_blocks[3] = { 1, 1, 0 };
        check_blocks[4] = { 1, 1, -1 };
        check_blocks[5] = { 1, 0, -1 };
        check_blocks[6] = { 1, -1, -1 };
        check_blocks[7] = { 1, -1, 0 };
        break;
    case Direction::top:
        check_blocks[0] = { -1, 1, 1 };
        check_blocks[1] = { 0, 1, 1 };
        check_blocks[2] = { 1, 1, 1 };
        check_blocks[3] = { 1, 0, 1 };
        check_blocks[4] = { 1, -1, 1 };
        check_blocks[5] = { 0, -1, 1 };
        check_blocks[6] = { -1, -1, 1 };
        check_blocks[7] = { -1, 0, 1 };
        break;
    case Direction::bottom:
        check_blocks[0] = { 1, 1, -1 };
        check_blocks[1] = { 0, 1, -1 };
        check_blocks[2] = { -1, 1, -1 };
        check_blocks[3] = { -1, 0, -1 };
        check_blocks[4] = { -1, -1, -1 };
        check_blocks[5] = { 0, -1, -1 };
        check_blocks[6] = { 1, -1, -1 };
        check_blocks[7] = { 1, 0, -1 };
        break;
    }

    std::array<uint8_t, 4> lighting = { 255, 255, 255, 255 };
    for (int i = 0; i < check_blocks.size(); i++) {
        mve::Vector3i check_block_local = local_block_pos + check_blocks[i];
        std::optional<uint8_t> check_block = data.block_at_relative(chunk_pos, check_block_local);
        if (!check_block.has_value()) {
            continue;
        }
        const float dark_fraction = 0.7f;
        if (check_block.value() != 0) {
            switch (i) {
            case 0:
                lighting[0] *= dark_fraction;
                break;
            case 1:
                lighting[0] *= dark_fraction;
                lighting[1] *= dark_fraction;
                break;
            case 2:
                lighting[1] *= dark_fraction;
                break;
            case 3:
                lighting[1] *= dark_fraction;
                lighting[2] *= dark_fraction;
                break;
            case 4:
                lighting[2] *= dark_fraction;
                break;
            case 5:
                lighting[2] *= dark_fraction;
                lighting[3] *= dark_fraction;
                break;
            case 6:
                lighting[3] *= dark_fraction;
                break;
            case 7:
                lighting[3] *= dark_fraction;
                lighting[0] *= dark_fraction;
                break;
            }
        }
    }

    return lighting;
}
