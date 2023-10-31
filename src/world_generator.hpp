#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "FastNoiseLite.h"

#include "mve/math/math.hpp"

#include <array>

#include "chunk_data.hpp"

class FastNoiseLite;
class ChunkColumn;

class WorldGenerator {
public:
    explicit WorldGenerator(int seed);

    bool generate_chunk(ChunkColumn& data, mve::Vector2i chunk_pos);

private:

    void generate_terrain(ChunkColumn& data, mve::Vector2i chunk_pos);

    // clang-format off
    const uint8_t c_tree_struct[7][5][5]
        = { { { 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0 },
              { 0, 0, 5, 0, 0 },
              { 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0 } },
            { { 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0 },
              { 0, 0, 5, 0, 0 },
              { 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0 } },
            { { 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0 },
              { 0, 0, 5, 0, 0 },
              { 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0 } },
            { { 0, 9, 9, 9, 0 },
              { 9, 9, 9, 9, 9 },
              { 9, 9, 5, 9, 9 },
              { 9, 9, 9, 9, 9 },
              { 0, 9, 9, 9, 0 } },
            { { 0, 9, 9, 9, 0 },
              { 9, 9, 9, 9, 9 },
              { 9, 9, 9, 9, 9 },
              { 9, 9, 9, 9, 9 },
              { 0, 9, 9, 9, 0 } },
            { { 0, 0, 0, 0, 0 },
              { 0, 9, 9, 9, 0 },
              { 0, 9, 9, 9, 0 },
              { 0, 9, 9, 9, 0 },
              { 0, 0, 0, 0, 0 } },
            { { 0, 0, 0, 0, 0 },
              { 0, 0, 9, 0, 0 },
              { 0, 9, 9, 9, 0 },
              { 0, 0, 9, 0, 0 },
              { 0, 0, 0, 0, 0 } },
            };
    // clang-format on

    std::unique_ptr<FastNoiseLite> m_noise_oct1;
    std::unique_ptr<FastNoiseLite> m_noise_oct2;
    std::unique_ptr<FastNoiseLite> m_noise_oct3;
    std::unique_ptr<FastNoiseLite> m_struct_noise;
};