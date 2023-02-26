#pragma once

#include <memory>
#include <stdint.h>

#include "FastNoiseLite.h"

#include "mve/math/math.hpp"

#include <array>

#include "chunk_data.hpp"

class FastNoiseLite;

class WorldGenerator {
public:
    explicit WorldGenerator(int seed);

    void generate_chunks(std::array<ChunkData*, 20>& chunks, mve::Vector2i chunk_pos);

private:
    std::unique_ptr<FastNoiseLite> m_noise_oct1;
    std::unique_ptr<FastNoiseLite> m_noise_oct2;
    std::unique_ptr<FastNoiseLite> m_noise_oct3;
};