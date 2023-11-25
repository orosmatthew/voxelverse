#include "world_generator.hpp"

#include "common.hpp"

#include "FastNoiseLite.h"
#include "lighting.hpp"
#include "world_data.hpp"

WorldGenerator::WorldGenerator(int seed)
    : m_noise_oct1(std::make_unique<FastNoiseLite>(seed))
    , m_noise_oct2(std::make_unique<FastNoiseLite>(seed + 1))
    , m_noise_oct3(std::make_unique<FastNoiseLite>(seed + 2))
    , m_struct_noise(std::make_unique<FastNoiseLite>(seed + 3))
{
    m_noise_oct1->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_noise_oct2->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_noise_oct3->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_struct_noise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_struct_noise->SetFrequency(1.0f);
}

void WorldGenerator::generate_chunk(WorldData& world_data, const mve::Vector2i chunk_pos) const
{
    if (world_data.contains_column(chunk_pos)) {
        if (world_data.chunk_column_data_at(chunk_pos).gen_level() >= ChunkColumn::generated) {
            return;
        }
    }
    for_2d({ -1, -1 }, { 2, 2 }, [&](const mve::Vector2i offset) { generate_trees(world_data, chunk_pos + offset); });
    ChunkColumn& column = world_data.chunk_column_data_at(chunk_pos);
    apply_sunlight(column);
    // for (int h = -10; h < 10; ++h) {
    //     world_data.propagate_light({ chunk_pos.x, chunk_pos.y, h });
    // }
    column.set_gen_level(ChunkColumn::GenLevel::generated);
}

void WorldGenerator::generate_terrain(ChunkColumn& data, mve::Vector2i chunk_pos) const
{
    if (data.gen_level() >= ChunkColumn::terrain) {
        return;
    }
    std::array<std::array<float, 16>, 16> heights {};

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            const mve::Vector2 scale_oct1 { 0.5f, 1.0f };
            const mve::Vector2 scale_oct2 { 1.0f, 1.0f };
            const mve::Vector2 scale_oct3 { 3.0f, 1.0f };
            const mve::Vector2 noise_pos { static_cast<float>(x + chunk_pos.x * 16),
                                           static_cast<float>(y + chunk_pos.y * 16) };
            const float height_oct1
                = m_noise_oct1->GetNoise(noise_pos.x * scale_oct1.x, noise_pos.y * scale_oct1.x) * 32.0f;
            const float height_oct2
                = m_noise_oct2->GetNoise(noise_pos.x * scale_oct2.x, noise_pos.y * scale_oct2.x) * 32.0f;
            const float height_oct3
                = m_noise_oct3->GetNoise(noise_pos.x * scale_oct3.x, noise_pos.y * scale_oct3.x) * 32.0f;
            const float height = 1.0f * height_oct1 + 0.5f * height_oct2 + 0.2f * height_oct3;
            heights[x][y] = height;
        }
    }

    for (int i = -10; i < 10; i++) {
        for_3d({ 0, 0, 0 }, { 16, 16, 16 }, [&](const mve::Vector3i pos) {
            if (const mve::Vector3i world_pos = block_local_to_world({ chunk_pos.x, chunk_pos.y, i }, pos);
                static_cast<float>(world_pos.z) < heights[pos.x][pos.y] - 4) {
                data.set_block(world_pos, 2);
            }
            else if (static_cast<float>(world_pos.z) < heights[pos.x][pos.y] - 1) {
                data.set_block(world_pos, 4);
            }
            else if (static_cast<float>(world_pos.z) < heights[pos.x][pos.y]) {
                data.set_block(world_pos, 1);
            }
            else {
                data.set_block(world_pos, 0);
            }
        });
    }
    data.set_gen_level(ChunkColumn::GenLevel::terrain);
}

void WorldGenerator::generate_trees(WorldData& world_data, mve::Vector2i chunk_pos) const
{
    for_2d({ -1, -1 }, { 2, 2 }, [&](const mve::Vector2i offset) {
        const mve::Vector2i neighbor_pos = chunk_pos + offset;
        if (!world_data.contains_column(neighbor_pos)) {
            world_data.create_or_load_chunk(neighbor_pos);
        }
        if (ChunkColumn& neighbor_column = world_data.chunk_column_data_at(neighbor_pos);
            neighbor_column.gen_level() < ChunkColumn::GenLevel::terrain) {
            generate_terrain(neighbor_column, neighbor_pos);
        }
    });
    ChunkColumn& column = world_data.chunk_column_data_at(chunk_pos);
    if (column.gen_level() >= ChunkColumn::GenLevel::trees) {
        return;
    }
    std::array<std::array<int, 16>, 16> heights {};
    for_2d({ 0, 0 }, { 16, 16 }, [&](const mve::Vector2i pos) {
        for (int h = 9 * 16; h > -10 * 16; h--) {
            if (column.get_block(mve::Vector3i(chunk_pos.x * 16 + pos.x, chunk_pos.y * 16 + pos.y, h)) == 1) {
                heights[pos.x][pos.y] = h;
                break;
            }
        }
    });
    for_2d({ 0, 0 }, { 16, 16 }, [&](mve::Vector2i pos) {
        const mve::Vector2i world_col_pos = block_local_to_world_col({ chunk_pos.x, chunk_pos.y }, { pos.x, pos.y });
        if (const float rand
            = m_struct_noise->GetNoise(static_cast<float>(world_col_pos.x), static_cast<float>(world_col_pos.y));
            rand <= 0.8f) {
            return;
        }
        const int height = heights[pos.x][pos.y];
        for_3d({ 0, 0, 0 }, { 5, 5, 7 }, [&](const mve::Vector3i struct_pos) {
            if (c_tree_struct[struct_pos.z][struct_pos.y][struct_pos.x] == 0) {
                return;
            }
            if (!is_block_height_world_valid(struct_pos.z + height + 1)) {
                return;
            }
            //            if (WorldData::is_block_pos_local_col(mve::Vector2i(pos.x + struct_pos.x - 2, pos.y +
            //            struct_pos.y - 2))) {
            //                int chunk_height = WorldData::chunk_height_from_block_height(std::floor(struct_pos.z +
            //                height + 1));
            mve::Vector3i world_pos = block_local_to_world(
                { chunk_pos.x, chunk_pos.y, 0 },
                { pos.x + struct_pos.x - 2, pos.y + struct_pos.y - 2, struct_pos.z + height + 1 });
            world_pos.z = struct_pos.z + height + 1;
            if (is_block_pos_local_col({ world_pos.x, world_pos.y })) {
                column.set_block(world_pos, c_tree_struct[struct_pos.z][struct_pos.y][struct_pos.x]);
            }
            else {
                world_data.set_block(world_pos, c_tree_struct[struct_pos.z][struct_pos.y][struct_pos.x]);
            }

            //            }
        });
    });
    column.set_gen_level(ChunkColumn::GenLevel::trees);
}
