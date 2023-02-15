#include "chunk_mesh.hpp"

#include "common.hpp"
#include "logger.hpp"
#include "mve/math/math.hpp"
#include "world_renderer.hpp"

ChunkMesh::ChunkMesh(mve::Vector3i chunk_pos, const WorldData& data, mve::Renderer& renderer)
    : m_mesh_buffers(create_buffers(chunk_pos, renderer, data))
    , m_chunk_pos(chunk_pos)
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

void ChunkMesh::calc_block_faces(
    uint8_t block_type,
    MeshData& mesh,
    const WorldData& world_data,
    const ChunkData& chunk_data,
    mve::Vector3i chunk_pos,
    mve::Vector3i local_pos,
    bool iterate_empty,
    const std::array<bool, 6>& directions)
{
    for (int f = 0; f < 6; f++) {
        if (!directions[f]) {
            continue;
        }
        Direction dir = static_cast<Direction>(f);
        mve::Vector3i adj_local_pos = local_pos + direction_vector(dir);
        std::optional<uint8_t> adj_block;
        if (WorldData::is_block_pos_local(adj_local_pos)) {
            adj_block = chunk_data.get_block(adj_local_pos);
        }
        else {
            adj_block = world_data.block_at(WorldData::block_local_to_world(chunk_pos, adj_local_pos));
        }
        uint8_t adj_block_type = 0;
        if (adj_block.has_value()) {
            adj_block_type = *adj_block;
        }
        if (iterate_empty) {
            if (adj_block_type != 0 && WorldData::is_block_pos_local(adj_local_pos)) {
                std::array<uint8_t, 4> face_lighting
                    = calc_face_lighting(world_data, chunk_data, chunk_pos, adj_local_pos, opposite_direction(dir));
                FaceData face = create_face_mesh(adj_block_type, adj_local_pos, opposite_direction(dir), face_lighting);
                add_face_to_mesh(mesh, face);
            }
        }
        else {
            if (adj_block_type == 0) {
                std::array<uint8_t, 4> face_lighting
                    = calc_face_lighting(world_data, chunk_data, chunk_pos, local_pos, dir);
                FaceData face = create_face_mesh(block_type, local_pos, dir, face_lighting);
                add_face_to_mesh(mesh, face);
            }
        }
    }
}

std::optional<ChunkMesh::MeshBuffers> ChunkMesh::create_buffers(
    mve::Vector3i chunk_pos, mve::Renderer& renderer, const WorldData& world_data)
{
    MeshData mesh;
    const ChunkData& chunk_data = world_data.chunk_data_at(chunk_pos);
    for_3d({ 0, 0, 0 }, { 16, 16, 16 }, [&](mve::Vector3i local_pos) {
        uint8_t block = chunk_data.get_block(local_pos);
        if (chunk_data.block_count() > (8 * 8 * 8)) {
            if (block != 0) {
                std::array<bool, 6> directions {};
                if (local_pos.x == 0) {
                    directions[static_cast<size_t>(Direction::left)] = true;
                }
                if (local_pos.x == 15) {
                    directions[static_cast<size_t>(Direction::right)] = true;
                }
                if (local_pos.y == 0) {
                    directions[static_cast<size_t>(Direction::front)] = true;
                }
                if (local_pos.y == 15) {
                    directions[static_cast<size_t>(Direction::back)] = true;
                }
                if (local_pos.z == 0) {
                    directions[static_cast<size_t>(Direction::bottom)] = true;
                }
                if (local_pos.z == 15) {
                    directions[static_cast<size_t>(Direction::top)] = true;
                }
                calc_block_faces(block, mesh, world_data, chunk_data, chunk_pos, local_pos, false, directions);
            }
            if (block == 0) {
                calc_block_faces(block, mesh, world_data, chunk_data, chunk_pos, local_pos, true);
            }
        }
        else {
            if (block != 0) {
                calc_block_faces(block, mesh, world_data, chunk_data, chunk_pos, local_pos, false);
            }
        }
    });
    //
    //    if (empty) {
    //        return {};
    //    }

    mve::VertexData vertex_data(WorldRenderer::vertex_layout());
    for (int i = 0; i < mesh.vertices.size(); i++) {
        vertex_data.push_back(mesh.vertices.at(i) + chunk_pos * 16.0f);
        vertex_data.push_back(mesh.colors.at(i));
        vertex_data.push_back(mesh.uvs.at(i));
    }

    if (vertex_data.vertex_count() == 0) {
        return {};
    }

    return MeshBuffers { renderer.create_vertex_buffer(vertex_data), renderer.create_index_buffer(mesh.indices) };
}
void ChunkMesh::draw(mve::Renderer& renderer) const
{
    if (m_mesh_buffers.has_value()) {
        renderer.bind_vertex_buffer(m_mesh_buffers->vertex_buffer);
        renderer.draw_index_buffer(m_mesh_buffers->index_buffer);
    }
}

ChunkMesh::FaceData ChunkMesh::create_face_mesh(
    uint8_t block_type, mve::Vector3 offset, Direction face, const std::array<uint8_t, 4>& lighting)
{
    FaceData data;
    QuadUVs uvs;
    switch (face) {
    case Direction::front:
        uvs = uvs_from_atlas({ 4, 4 }, block_uv(block_type, Direction::front));
        data.vertices[0] = mve::Vector3(-0.5f, -0.5f, 0.5f) + offset;
        data.vertices[1] = mve::Vector3(0.5f, -0.5f, 0.5f) + offset;
        data.vertices[2] = mve::Vector3(0.5f, -0.5f, -0.5f) + offset;
        data.vertices[3] = mve::Vector3(-0.5f, -0.5f, -0.5f) + offset;
        break;
    case Direction::back:
        uvs = uvs_from_atlas({ 4, 4 }, block_uv(block_type, Direction::back));
        data.vertices[0] = mve::Vector3(0.5f, 0.5f, 0.5f) + offset;
        data.vertices[1] = mve::Vector3(-0.5f, 0.5f, 0.5f) + offset;
        data.vertices[2] = mve::Vector3(-0.5f, 0.5f, -0.5f) + offset;
        data.vertices[3] = mve::Vector3(0.5f, 0.5f, -0.5f) + offset;
        break;
    case Direction::left:
        uvs = uvs_from_atlas({ 4, 4 }, block_uv(block_type, Direction::left));
        data.vertices[0] = mve::Vector3(-0.5f, 0.5f, 0.5f) + offset;
        data.vertices[1] = mve::Vector3(-0.5f, -0.5f, 0.5f) + offset;
        data.vertices[2] = mve::Vector3(-0.5f, -0.5f, -0.5f) + offset;
        data.vertices[3] = mve::Vector3(-0.5f, 0.5f, -0.5f) + offset;
        break;
    case Direction::right:
        uvs = uvs_from_atlas({ 4, 4 }, block_uv(block_type, Direction::right));
        data.vertices[0] = mve::Vector3(0.5f, -0.5f, 0.5f) + offset;
        data.vertices[1] = mve::Vector3(0.5f, 0.5f, 0.5f) + offset;
        data.vertices[2] = mve::Vector3(0.5f, 0.5f, -0.5f) + offset;
        data.vertices[3] = mve::Vector3(0.5f, -0.5f, -0.5f) + offset;
        break;
    case Direction::top:
        uvs = uvs_from_atlas({ 4, 4 }, block_uv(block_type, Direction::top));
        data.vertices[0] = mve::Vector3(-0.5f, 0.5f, 0.5f) + offset;
        data.vertices[1] = mve::Vector3(0.5f, 0.5f, 0.5f) + offset;
        data.vertices[2] = mve::Vector3(0.5f, -0.5f, 0.5f) + offset;
        data.vertices[3] = mve::Vector3(-0.5f, -0.5f, 0.5f) + offset;
        break;
    case Direction::bottom:
        uvs = uvs_from_atlas({ 4, 4 }, block_uv(block_type, Direction::bottom));
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
    const WorldData& data,
    const ChunkData& chunk_data,
    mve::Vector3i chunk_pos,
    mve::Vector3i local_block_pos,
    Direction dir)
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
        std::optional<uint8_t> check_block;
        if (WorldData::is_block_pos_local(check_block_local)) {
            check_block = chunk_data.get_block(check_block_local);
        }
        else {
            check_block = data.block_at_relative(chunk_pos, check_block_local);
        }
        if (!check_block.has_value()) {
            continue;
        }
        const float dark_fraction = 0.8f;
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
