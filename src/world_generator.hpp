#pragma once

#include <memory>
#include <stdint.h>

#include "math/math.hpp"

class FastNoiseLite;

class WorldGenerator {
public:
    WorldGenerator(int seed);

    uint8_t get_block(mve::Vector3i pos, bool terrain) const;

private:
    std::unique_ptr<FastNoiseLite> m_noise;
};