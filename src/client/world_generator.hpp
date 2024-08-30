#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include <FastNoiseLite.h>

#include "common.hpp"

#include <nnm/nnm.hpp>

class FastNoiseLite;
class ChunkColumn;
class WorldData;

class WorldGenerator {
public:
    explicit WorldGenerator(int seed);

    void generate_chunk(WorldData& world_data, nnm::Vector2i chunk_pos) const;

private:
    void generate_terrain(ChunkColumn& data, nnm::Vector2i chunk_pos) const;

    void generate_trees(WorldData& world_data, nnm::Vector2i chunk_pos) const;

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