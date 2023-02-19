#pragma once

#include <memory>
#include <stdint.h>

#include "FastNoiseLite.h"

#include "mve/math/math.hpp"

class FastNoiseLite;

class WorldGenerator {
public:
    WorldGenerator(int seed);

    uint8_t get_block(mve::Vector3i pos) const;

private:
    std::unique_ptr<FastNoiseLite> m_noise_oct1;
    std::unique_ptr<FastNoiseLite> m_noise_oct2;
    std::unique_ptr<FastNoiseLite> m_noise_oct3;
};