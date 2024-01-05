#include "chunk_mesh.hpp"

#include <mve/math/math.hpp>

#include "chunk_data.hpp"
#include "common.hpp"
#include "world_data.hpp"
#include "world_renderer.hpp"

void combine_mesh_data(ChunkMeshData& data, const ChunkMeshData& other)
{
    const uint32_t indices_offset = data.vertices.size();
    for (int i = 0; i < other.vertices.size(); i++) {
        data.vertices.push_back(other.vertices[i]);
        data.colors.push_back(other.colors[i]);
        data.uvs.push_back(other.uvs[i]);
    }

    for (const unsigned int index : other.indices) {
        data.indices.push_back(index + indices_offset);
    }
}

std::array<uint8_t, 4> calc_chunk_face_lighting(
    const WorldData& data,
    const ChunkData& chunk_data,
    const mve::Vector3i chunk_pos,
    const mve::Vector3i local_block_pos,
    const Direction dir)
{
    const mve::Vector3i world_pos = block_local_to_world(chunk_pos, local_block_pos);

    uint8_t base_lighting = 0;

    // format of check_blocks
    //      0 | 1 | 2
    //      ---------
    //      7 |   | 3
    //      ---------
    //      6 | 5 | 4
    std::array<mve::Vector3i, 8> check_blocks;
    switch (dir) {
    case Direction::front:
        base_lighting = data.lighting_at(world_pos + mve::Vector3i(0, -1, 0)).value();
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
        base_lighting = data.lighting_at(world_pos + mve::Vector3i(0, 1, 0)).value();
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
        base_lighting = data.lighting_at(world_pos + mve::Vector3i(-1, 0, 0)).value();
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
        base_lighting = data.lighting_at(world_pos + mve::Vector3i(1, 0, 0)).value();
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
        base_lighting = data.lighting_at(world_pos + mve::Vector3i(0, 0, 1)).value();
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
        base_lighting = data.lighting_at(world_pos + mve::Vector3i(0, 0, -1)).value();
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

    VV_DEB_ASSERT(base_lighting >= 0 && base_lighting <= 15, "[ChunkMesh] Base lighting is not between 0 and 15")

    // format of adj_light
    //      0 | 1 |        | 0 | 1      |   |        |   |
    //      ---------    ---------    ---------    ---------
    //      2 | 3 |        | 2 | 3      | 0 | 1    0 | 1 |
    //      ---------    ---------    ---------    ---------
    //        |   |        |   |        | 2 | 3    2 | 3 |
    std::array<std::array<std::optional<int>, 4>, 4> adj_light {
        { { std::nullopt, std::nullopt, std::nullopt, base_lighting },
          { std::nullopt, std::nullopt, base_lighting, std::nullopt },
          { base_lighting, std::nullopt, std::nullopt, std::nullopt },
          { std::nullopt, base_lighting, std::nullopt, std::nullopt } }
    };

    for (int i = 0; i < check_blocks.size(); ++i) {
        const mve::Vector3i check_block_local = local_block_pos + check_blocks[i];
        std::optional<uint8_t> check_block;
        if (is_block_pos_local(check_block_local)) {
            check_block = chunk_data.get_block(check_block_local);
        }
        else {
            check_block = data.block_at_relative(chunk_pos, check_block_local);
        }
        if (!check_block.has_value()) {
            continue;
        }
        if (is_transparent(*check_block)) {
            std::optional<uint8_t> block_light;
            if (is_block_pos_local(check_block_local)) {
                block_light = chunk_data.lighting_at(check_block_local);
            }
            else {
                block_light = data.lighting_at_relative(chunk_pos, check_block_local);
            }
            if (block_light.has_value()) {
                switch (i) {
                case 0:
                    adj_light[0][0] = *block_light;
                    break;
                case 1:
                    adj_light[0][1] = *block_light;
                    adj_light[1][0] = *block_light;
                    break;
                case 2:
                    adj_light[1][1] = *block_light;
                    break;
                case 3:
                    adj_light[1][3] = *block_light;
                    adj_light[2][1] = *block_light;
                    break;
                case 4:
                    adj_light[2][3] = *block_light;
                    break;
                case 5:
                    adj_light[2][2] = *block_light;
                    adj_light[3][3] = *block_light;
                    break;
                case 6:
                    adj_light[3][2] = *block_light;
                    break;
                case 7:
                    adj_light[0][2] = *block_light;
                    adj_light[3][0] = *block_light;
                    break;
                default:
                    VV_REL_ASSERT(false, "Unreachable")
                }
            }
        }
    }

    auto avg_light_opts = [](const std::array<std::optional<int>, 4>& arr) -> uint8_t {
        int total = 0;
        int count = 0;
        for (const std::optional<int> i : arr) {
            if (i.has_value()) {
                total += *i;
                count++;
            }
        }
        const int val = count == 0 ? 0 : total / count;
        return static_cast<uint8_t>(mve::clamp(val * 16, 0, 255));
    };

    std::array lighting
        = { avg_light_opts(adj_light[0]),
            avg_light_opts(adj_light[1]),
            avg_light_opts(adj_light[2]),
            avg_light_opts(adj_light[3]) };

    for (int i = 0; i < check_blocks.size(); i++) {
        const mve::Vector3i check_block_local = local_block_pos + check_blocks[i];
        std::optional<uint8_t> check_block;
        if (is_block_pos_local(check_block_local)) {
            check_block = chunk_data.get_block(check_block_local);
        }
        else {
            check_block = data.block_at_relative(chunk_pos, check_block_local);
        }
        if (!check_block.has_value()) {
            continue;
        }
        constexpr float occlusion_factor = 0.8f;
        static_assert(255 - static_cast<int>(occlusion_factor) * 3 > 0);
        if (check_block.value() != 0) {
            switch (i) {
            case 0:
                lighting[0] = static_cast<uint8_t>(static_cast<float>(lighting[0]) * occlusion_factor);
                break;
            case 1:
                lighting[0] = static_cast<uint8_t>(static_cast<float>(lighting[0]) * occlusion_factor);
                lighting[1] = static_cast<uint8_t>(static_cast<float>(lighting[1]) * occlusion_factor);
                break;
            case 2:
                lighting[1] = static_cast<uint8_t>(static_cast<float>(lighting[1]) * occlusion_factor);
                break;
            case 3:
                lighting[1] = static_cast<uint8_t>(static_cast<float>(lighting[1]) * occlusion_factor);
                lighting[2] = static_cast<uint8_t>(static_cast<float>(lighting[2]) * occlusion_factor);
                break;
            case 4:
                lighting[2] = static_cast<uint8_t>(static_cast<float>(lighting[2]) * occlusion_factor);
                break;
            case 5:
                lighting[2] = static_cast<uint8_t>(static_cast<float>(lighting[2]) * occlusion_factor);
                lighting[3] = static_cast<uint8_t>(static_cast<float>(lighting[3]) * occlusion_factor);
                break;
            case 6:
                lighting[3] = static_cast<uint8_t>(static_cast<float>(lighting[3]) * occlusion_factor);
                break;
            case 7:
                lighting[3] = static_cast<uint8_t>(static_cast<float>(lighting[3]) * occlusion_factor);
                lighting[0] = static_cast<uint8_t>(static_cast<float>(lighting[0]) * occlusion_factor);
                break;
            default:
                VV_REL_ASSERT(false, "Unreachable")
            }
        }
    }

    return lighting;
}

ChunkFaceData create_chunk_face_mesh(
    const uint8_t block_type, const mve::Vector3 offset, const Direction face, const std::array<uint8_t, 4>& lighting)
{
    ChunkFaceData data;
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
        VV_REL_ASSERT(false, "Unreachable")
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

void add_face_to_mesh(ChunkMeshData& data, const ChunkFaceData& face)
{
    const uint32_t indices_offset = data.vertices.size();
    for (int i = 0; i < face.vertices.size(); i++) {
        data.vertices.push_back(face.vertices[i]);
        data.colors.push_back(face.colors[i]);
        data.uvs.push_back(face.uvs[i]);
    }

    for (const unsigned int index : face.indices) {
        data.indices.push_back(index + indices_offset);
    }
}

void calc_chunk_block_faces(
    const uint8_t block_type,
    ChunkMeshData& mesh,
    const WorldData& world_data,
    const ChunkData& chunk_data,
    const mve::Vector3i chunk_pos,
    const mve::Vector3i local_pos,
    const bool iterate_empty,
    const std::array<bool, 6>& directions = { true, true, true, true, true, true })
{
    for (int f = 0; f < 6; f++) {
        if (!directions[f]) {
            continue;
        }
        const auto dir = static_cast<Direction>(f);
        const mve::Vector3i adj_local_pos = local_pos + direction_vector(dir);
        std::optional<uint8_t> adj_block;
        if (is_block_pos_local(adj_local_pos)) {
            adj_block = chunk_data.get_block(adj_local_pos);
        }
        else {
            adj_block = world_data.block_at(block_local_to_world(chunk_pos, adj_local_pos));
        }
        uint8_t adj_block_type = 0;
        if (adj_block.has_value()) {
            adj_block_type = *adj_block;
        }
        if (iterate_empty) {
            if (adj_block_type != 0 && is_block_pos_local(adj_local_pos)) {
                std::array<uint8_t, 4> face_lighting = calc_chunk_face_lighting(
                    world_data, chunk_data, chunk_pos, adj_local_pos, opposite_direction(dir));
                ChunkFaceData face = create_chunk_face_mesh(
                    adj_block_type, mve::Vector3(adj_local_pos), opposite_direction(dir), face_lighting);
                add_face_to_mesh(mesh, face);
            }
        }
        else {
            if (adj_block_type == 0 || is_transparent(adj_block_type)) {
                std::array<uint8_t, 4> face_lighting
                    = calc_chunk_face_lighting(world_data, chunk_data, chunk_pos, local_pos, dir);
                ChunkFaceData face = create_chunk_face_mesh(block_type, mve::Vector3(local_pos), dir, face_lighting);
                add_face_to_mesh(mesh, face);
            }
        }
    }
}

std::optional<ChunkBufferData> create_chunk_buffer_data(const mve::Vector3i chunk_pos, const WorldData& world_data)
{
    ChunkMeshData mesh;
    const ChunkData& chunk_data = world_data.chunk_data_at(chunk_pos);
    for_3d({ 0, 0, 0 }, { 16, 16, 16 }, [&](const mve::Vector3i local_pos) {
        const uint8_t block = chunk_data.get_block(local_pos);
        if (chunk_data.block_count() > 8 * 8 * 8) {
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
                calc_chunk_block_faces(block, mesh, world_data, chunk_data, chunk_pos, local_pos, false, directions);
            }
            if (block == 0 || is_transparent(block)) {
                calc_chunk_block_faces(block, mesh, world_data, chunk_data, chunk_pos, local_pos, true);
            }
        }
        else {
            if (block != 0) {
                calc_chunk_block_faces(block, mesh, world_data, chunk_data, chunk_pos, local_pos, false);
            }
        }
    });

    if (mesh.vertices.empty()) {
        return {};
    }

    mve::VertexData vertex_data(WorldRenderer::vertex_layout());
    for (int i = 0; i < mesh.vertices.size(); i++) {
        vertex_data.push_back(mesh.vertices.at(i) + mve::Vector3(chunk_pos) * 16.0f);
        vertex_data.push_back(mesh.colors.at(i));
        vertex_data.push_back(mesh.uvs.at(i));
        vertex_data.push_back(1.0f);
    }
    return ChunkBufferData {
        .chunk_pos = chunk_pos, .vertex_data = std::move(vertex_data), .index_data = std::move(mesh.indices)
    };
}
