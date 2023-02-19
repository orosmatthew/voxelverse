#include "world_generator.hpp"

#include "common.hpp"

#include "FastNoiseLite.h"

WorldGenerator::WorldGenerator(int seed)
    : m_noise_oct1(std::make_unique<FastNoiseLite>(seed))
    , m_noise_oct2(std::make_unique<FastNoiseLite>(seed + 1))
    , m_noise_oct3(std::make_unique<FastNoiseLite>(seed + 2))
{
    m_noise_oct1->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_noise_oct2->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_noise_oct3->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
}

void WorldGenerator::generate_chunks(std::array<ChunkData*, 20>& chunks, mve::Vector2i chunk_pos)
{
    std::array<std::array<float, 16>, 16> heights;

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            mve::Vector2 scale_oct1 { 0.5f, 1.0f };
            mve::Vector2 scale_oct2 { 1.0f, 1.0f };
            mve::Vector2 scale_oct3 { 3.0f, 1.0f };
            mve::Vector2 noise_pos { static_cast<float>(x + chunk_pos.x * 16),
                                     static_cast<float>(y + chunk_pos.y * 16) };
            float height_oct1 = m_noise_oct1->GetNoise(noise_pos.x * scale_oct1.x, noise_pos.y * scale_oct1.x) * 32.0f;
            float height_oct2 = m_noise_oct2->GetNoise(noise_pos.x * scale_oct2.x, noise_pos.y * scale_oct2.x) * 32.0f;
            int height_oct3 = m_noise_oct3->GetNoise(noise_pos.x * scale_oct3.x, noise_pos.y * scale_oct3.x) * 32.0f;
            float height = (1.0f * height_oct1 + 0.5f * height_oct2 + 0.2f * height_oct3);
            heights[x][y] = height;
        }
    }

    for (ChunkData* data : chunks) {
        for_3d({ 0, 0, 0 }, { 16, 16, 16 }, [&](mve::Vector3i pos) {
            int world_z = pos.z + data->position().z * 16;
            if (world_z < heights[pos.x][pos.y] - 4) {
                data->set_block(pos, 2);
            }
            else if (world_z < heights[pos.x][pos.y] - 1) {
                data->set_block(pos, 4);
            }
            else if (world_z < heights[pos.x][pos.y]) {
                data->set_block(pos, 1);
            }
            else {
                data->set_block(pos, 0);
            }
        });
    }
}
