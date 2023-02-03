#include "world_generator.hpp"

#include "FastNoiseLite.h"

uint8_t WorldGenerator::get_block(mve::Vector3i pos) const
{
    return static_cast<uint8_t>(
        m_noise->GetNoise(
            static_cast<float>(pos.x) * 10, static_cast<float>(pos.y) * 10, static_cast<float>(pos.z) * 10)
        > 0.5f);
}

WorldGenerator::WorldGenerator(int seed)
    : m_noise(std::make_unique<FastNoiseLite>(seed))
{
    m_noise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
}
