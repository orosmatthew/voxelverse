#pragma once

#include <stdint.h>

#include "mve/math/math.hpp"

enum class Direction { front = 0, back, left, right, top, bottom };

inline mve::Vector3i direction_vector(Direction dir)
{
    switch (dir) {
    case Direction::front:
        return { 0, -1, 0 };
    case Direction::back:
        return { 0, 1, 0 };
    case Direction::left:
        return { -1, 0, 0 };
    case Direction::right:
        return { 1, 0, 0 };
    case Direction::top:
        return { 0, 0, 1 };
    case Direction::bottom:
        return { 0, 0, -1 };
    default:
        return { 0, 0, 0 };
    }
}

inline Direction opposite_direction(Direction dir)
{
    switch (dir) {
    case Direction::front:
        return Direction::back;
    case Direction::back:
        return Direction::front;
    case Direction::left:
        return Direction::right;
    case Direction::right:
        return Direction::left;
    case Direction::top:
        return Direction::bottom;
    case Direction::bottom:
        return Direction::top;
    default:
        return Direction::front;
    }
}

class ChunkData {
public:
    ChunkData(mve::Vector3i chunk_pos);

    mve::Vector3i position() const;

    void set_block(mve::Vector3i pos, uint8_t type);
    inline uint8_t get_block(mve::Vector3i pos) const
    {
        return m_block_data[pos.x][pos.y][pos.z];
    }

    inline int block_count() const
    {
        return m_block_count;
    }

    bool in_bounds(mve::Vector3i pos) const;

private:
    static const int sc_chunk_size = 16;
    mve::Vector3i m_pos;
    uint8_t m_block_data[sc_chunk_size][sc_chunk_size][sc_chunk_size] = { 0 };
    int m_block_count = 0;
};