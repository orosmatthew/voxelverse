#pragma once

#include <memory>
#include <stdint.h>

#include "math/vector3i.hpp"

class FastNoiseLite;

class WorldGenerator {
public:
    WorldGenerator(int seed);

    uint8_t get_block(mve::Vector3i pos) const;

private:
    std::unique_ptr<FastNoiseLite> m_noise;
};