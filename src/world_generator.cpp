#include "world_generator.hpp"

#include "common.hpp"

#include "FastNoiseLite.h"
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

void WorldGenerator::generate_chunks(ChunkColumn& data, mve::Vector2i chunk_pos)
{
    std::array<std::array<float, 16>, 16> heights {};

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            mve::Vector2 scale_oct1 { 0.5f, 1.0f };
            mve::Vector2 scale_oct2 { 1.0f, 1.0f };
            mve::Vector2 scale_oct3 { 3.0f, 1.0f };
            mve::Vector2 noise_pos { static_cast<float>(x + chunk_pos.x * 16),
                                     static_cast<float>(y + chunk_pos.y * 16) };
            float height_oct1 = m_noise_oct1->GetNoise(noise_pos.x * scale_oct1.x, noise_pos.y * scale_oct1.x) * 32.0f;
            float height_oct2 = m_noise_oct2->GetNoise(noise_pos.x * scale_oct2.x, noise_pos.y * scale_oct2.x) * 32.0f;
            float height_oct3 = m_noise_oct3->GetNoise(noise_pos.x * scale_oct3.x, noise_pos.y * scale_oct3.x) * 32.0f;
            float height = (1.0f * height_oct1 + 0.5f * height_oct2 + 0.2f * height_oct3);
            heights[x][y] = height;
        }
    }

    for (int i = -10; i < 10; i++) {
        for_3d({ 0, 0, 0 }, { 16, 16, 16 }, [&](mve::Vector3i pos) {
            mve::Vector3i world_pos = block_local_to_world({ chunk_pos.x, chunk_pos.y, i }, pos);
            if (static_cast<float>(world_pos.z) < heights[pos.x][pos.y] - 4) {
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

    for_2d({ 0, 0 }, { 16, 16 }, [&](mve::Vector2i pos) {
        mve::Vector2i world_col_pos
            = WorldData::block_local_to_world_col({ chunk_pos.x, chunk_pos.y }, { pos.x, pos.y });
        float rand = m_struct_noise->GetNoise(static_cast<float>(world_col_pos.x), static_cast<float>(world_col_pos.y));
        if (rand <= 0.8f) {
            return;
        }
        int height = std::floor(heights[pos.x][pos.y]);
        for_3d({ 0, 0, 0 }, { 5, 5, 7 }, [&](mve::Vector3i struct_pos) {
            if (c_tree_struct[struct_pos.z][struct_pos.y][struct_pos.x] == 0) {
                return;
            }
            if (!WorldData::is_block_height_world_valid(struct_pos.z + height + 1)) {
                return;
            }
            if (WorldData::is_block_pos_local_col(mve::Vector2i(pos.x + struct_pos.x - 2, pos.y + struct_pos.y - 2))) {
                //                int chunk_height = WorldData::chunk_height_from_block_height(std::floor(struct_pos.z +
                //                height + 1));
                mve::Vector3i world_pos = block_local_to_world(
                    { chunk_pos.x, chunk_pos.y, 0 },
                    { pos.x + struct_pos.x - 2, pos.y + struct_pos.y - 2, struct_pos.z + height + 1 });
                world_pos.z = struct_pos.z + height + 1;
                data.set_block(world_pos, c_tree_struct[struct_pos.z][struct_pos.y][struct_pos.x]);
            }
        });
    });
}
