#pragma once

#include <stdint.h>

#include "math/vector3i.hpp"

enum class Direction { front = 0, back, left, right, top, bottom };

mve::Vector3i direction_vector(Direction dir);

Direction opposite_direction(Direction dir);

class ChunkData {
public:
    ChunkData();

    void set_block(mve::Vector3i pos, uint8_t type);
    uint8_t get_block(mve::Vector3i pos) const;

    bool in_bounds(mve::Vector3i pos) const;

private:
    static const int sc_chunk_size = 16;
    uint8_t m_block_data[sc_chunk_size][sc_chunk_size][sc_chunk_size] = { 0 };
};