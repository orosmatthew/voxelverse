#include "chunk_mesh.hpp"

#include "common.hpp"
#include "mve/math/math.hpp"
#include "world_renderer.hpp"

ChunkMesh::ChunkMesh(mve::Vector3i chunk_pos)
    : m_chunk_pos(chunk_pos)
{
    //    create_mesh_data(chunk_pos, data);
}

void ChunkMesh::combine_mesh_data(MeshData& data, const MeshData& other)
{
    uint32_t indices_offset = data.vertices.size();
    for (int i = 0; i < other.vertices.size(); i++) {
        data.vertices.push_back(other.vertices[i]);
        data.colors.push_back(other.colors[i]);
        data.uvs.push_back(other.uvs[i]);
    }

    for (unsigned int index : other.indices) {
        data.indices.push_back(index + indices_offset);
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
        auto dir = static_cast<Direction>(f);
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
                FaceData face = create_face_mesh(
                    adj_block_type, mve::Vector3(adj_local_pos), opposite_direction(dir), face_lighting);
                add_face_to_mesh(mesh, face);
            }
        }
        else {
            if (adj_block_type == 0 || is_transparent(adj_block_type)) {
                std::array<uint8_t, 4> face_lighting
                    = calc_face_lighting(world_data, chunk_data, chunk_pos, local_pos, dir);
                FaceData face = create_face_mesh(block_type, mve::Vector3(local_pos), dir, face_lighting);
                add_face_to_mesh(mesh, face);
            }
        }
    }
}

void ChunkMesh::create_mesh_data(mve::Vector3i chunk_pos, const WorldData& world_data)
{
    m_chunk_pos = chunk_pos;
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
            if (block == 0 || is_transparent(block)) {
                calc_block_faces(block, mesh, world_data, chunk_data, chunk_pos, local_pos, true);
            }
        }
        else {
            if (block != 0) {
                calc_block_faces(block, mesh, world_data, chunk_data, chunk_pos, local_pos, false);
            }
        }
    });

    if (mesh.vertices.empty()) {
        return;
    }

    mve::VertexData vertex_data(WorldRenderer::vertex_layout());
    for (int i = 0; i < mesh.vertices.size(); i++) {
        vertex_data.push_back(mesh.vertices.at(i) + mve::Vector3(m_chunk_pos) * 16.0f);
        vertex_data.push_back(mesh.colors.at(i));
        vertex_data.push_back(mesh.uvs.at(i));
        vertex_data.push_back(1.0f);
    }

    m_vertex_data = { std::move(vertex_data), std::move(mesh.indices) };
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
        MVE_ASSERT(false, "Unreachable")
    }
    data.uvs[0] = uvs.top_left;
    data.uvs[1] = uvs.top_right;
    data.uvs[2] = uvs.bottom_right;
    data.uvs[3] = uvs.bottom_left;
    data.colors[0] = { static_cast<float>(lighting[0]) / 255.0f,
                       static_cast<float>(lighting[0]) / 255.0f,
                       static_cast<float>(lighting[0]) / 255.0f };
    data.colors[1] = { static_cast<float>(lighting[1]) / 255.0f,
                       static_cast<float>(lighting[1]) / 255.0f,
                       static_cast<float>(lighting[1]) / 255.0f };
    data.colors[2] = { static_cast<float>(lighting[2]) / 255.0f,
                       static_cast<float>(lighting[2]) / 255.0f,
                       static_cast<float>(lighting[2]) / 255.0f };
    data.colors[3] = { static_cast<float>(lighting[3]) / 255.0f,
                       static_cast<float>(lighting[3]) / 255.0f,
                       static_cast<float>(lighting[3]) / 255.0f };
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

    for (unsigned int index : face.indices) {
        data.indices.push_back(index + indices_offset);
    }
}

std::array<uint8_t, 4> ChunkMesh::calc_face_lighting(
    const WorldData& data,
    const ChunkData& chunk_data,
    mve::Vector3i chunk_pos,
    mve::Vector3i local_block_pos,
    Direction dir)
{
    uint8_t base_lighting;
    //    mve::Vector3i world_pos = WorldData::block_local_to_world(chunk_pos, local_block_pos);
    std::array<mve::Vector3i, 8> check_blocks;
    switch (dir) {
    case Direction::front:
        base_lighting = 15; // base_lighting = data.lighting_at(world_pos + mve::Vector3i(0, -1, 0)).value();
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
        base_lighting = 15; // base_lighting = data.lighting_at(world_pos + mve::Vector3i(0, 1, 0)).value();
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
        base_lighting = 15; // base_lighting = data.lighting_at(world_pos + mve::Vector3i(-1, 0, 0)).value();
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
        base_lighting = 15; // base_lighting = data.lighting_at(world_pos + mve::Vector3i(1, 0, 0)).value();
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
        base_lighting = 15; // base_lighting = data.lighting_at(world_pos + mve::Vector3i(0, 0, 1)).value();
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
        base_lighting = 15; // base_lighting = data.lighting_at(world_pos + mve::Vector3i(0, 0, -1)).value();
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

    MVE_VAL_ASSERT(base_lighting >= 0 && base_lighting <= 15, "[ChunkMesh] Base lighting is not between 0 and 15")

    base_lighting = static_cast<uint8_t>(mve::clamp(base_lighting * 16, 0, 255));
    std::array<uint8_t, 4> lighting = { base_lighting, base_lighting, base_lighting, base_lighting };

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
        const uint8_t occlusion_factor = 35;
        static_assert(255 - static_cast<int>(occlusion_factor) * 3 > 0);
        if (check_block.value() != 0) {
            switch (i) {
            case 0:
                lighting[0] -= occlusion_factor;
                break;
            case 1:
                lighting[0] -= occlusion_factor;
                lighting[1] -= occlusion_factor;
                break;
            case 2:
                lighting[1] -= occlusion_factor;
                break;
            case 3:
                lighting[1] -= occlusion_factor;
                lighting[2] -= occlusion_factor;
                break;
            case 4:
                lighting[2] -= occlusion_factor;
                break;
            case 5:
                lighting[2] -= occlusion_factor;
                lighting[3] -= occlusion_factor;
                break;
            case 6:
                lighting[3] -= occlusion_factor;
                break;
            case 7:
                lighting[3] -= occlusion_factor;
                lighting[0] -= occlusion_factor;
                break;
            default:
                MVE_ASSERT(false, "Unreachable")
            }
        }
    }

    return lighting;
}
void ChunkMesh::create_buffers(mve::Renderer& renderer)
{
    if (!m_vertex_data.has_value()) {
        return;
    }
    m_mesh_buffers
        = { renderer.create_vertex_buffer(m_vertex_data->first), renderer.create_index_buffer(m_vertex_data->second) };
    m_vertex_data.reset();
}
ChunkMesh::ChunkMesh() = default;
