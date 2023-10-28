#pragma once

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

    void generate_chunks(ChunkColumn& data, mve::Vector2i chunk_pos);

private:
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