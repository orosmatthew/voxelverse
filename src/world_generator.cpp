#include "world_generator.hpp"

#include "FastNoiseLite.h"

uint8_t WorldGenerator::get_block(mve::Vector3i pos) const
{
    mve::Vector2 scale_oct1 { 0.5f, 1.0f };
    mve::Vector2 scale_oct2 { 1.0f, 1.0f };
    mve::Vector2 scale_oct3 { 3.0f, 1.0f };
    float height_oct1
        = m_noise_oct1->GetNoise(static_cast<float>(pos.x) * scale_oct1.x, static_cast<float>(pos.y) * scale_oct1.x)
        * 32.0f;
    float height_oct2
        = m_noise_oct2->GetNoise(static_cast<float>(pos.x) * scale_oct2.x, static_cast<float>(pos.y) * scale_oct2.x)
        * 32.0f;
    int height_oct3
        = m_noise_oct3->GetNoise(static_cast<float>(pos.x) * scale_oct3.x, static_cast<float>(pos.y) * scale_oct3.x)
        * 32.0f;
    float height = (1.0f * height_oct1 + 0.5f * height_oct2 + 0.2f * height_oct3);
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

WorldGenerator::WorldGenerator(int seed)
    : m_noise_oct1(std::make_unique<FastNoiseLite>(seed))
    , m_noise_oct2(std::make_unique<FastNoiseLite>(seed + 1))
    , m_noise_oct3(std::make_unique<FastNoiseLite>(seed + 2))
{
    m_noise_oct1->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_noise_oct2->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_noise_oct3->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
}
