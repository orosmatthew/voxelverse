#include "world_generator.hpp"

#include "FastNoiseLite.h"

uint8_t WorldGenerator::get_block(mve::Vector3i pos, bool terrain) const
{
    float scale = 2.5f;
    if (terrain) {
        int height
            = m_noise->GetNoise(static_cast<float>(pos.x) * scale, static_cast<float>(pos.y) * scale) * 16.0f - 32;
        if (pos.z < height - 4) {
            return 2;
        }
        else if (pos.z < height - 1) {
            return 4;
        }
        else if (pos.z < height) {
            return 1;
        }
        else {
            return 0;
        }
    }
    else
        return static_cast<uint8_t>(
            m_noise->GetNoise(
                static_cast<float>(pos.x) * scale, static_cast<float>(pos.y) * scale, static_cast<float>(pos.z) * scale)
            > 0.5f);
}

WorldGenerator::WorldGenerator(int seed)
    : m_noise(std::make_unique<FastNoiseLite>(seed))
{
    m_noise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
}
