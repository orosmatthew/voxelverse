#pragma once

#include <array>
#include <cstdint>
#include <functional>

#include <cereal/types/vector.hpp>

#include "common.hpp"
#include "mve/math/math.hpp"

#include <mve/common.hpp>

inline mve::Vector3i direction_vector(const Direction dir)
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

inline Direction opposite_direction(const Direction dir)
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
    ChunkData();

    explicit ChunkData(mve::Vector3i chunk_pos);

    void reset_lighting(const uint8_t value = 0)
    {
        std::ranges::fill(m_lighting_data, value);
    }

    [[nodiscard]] mve::Vector3i position() const
    {
        return m_pos;
    }

    void set_block(mve::Vector3i pos, uint8_t type);
    [[nodiscard]] uint8_t get_block(const mve::Vector3i pos) const
    {
        MVE_VAL_ASSERT(is_block_pos_local(pos), "[ChunkData] Invalid local block position");
        return m_block_data[index(pos)];
    }

    void set_lighting(const mve::Vector3i pos, const uint8_t val)
    {
        MVE_VAL_ASSERT(is_block_pos_local(pos), "[ChunkData] Invalid local block position");
        MVE_VAL_ASSERT(val <= 15, "[ChunkData] Lighting is not between 0 and 15")
        m_lighting_data[index(pos)] = val;
    }

    [[nodiscard]] uint8_t lighting_at(const mve::Vector3i pos) const
    {
        MVE_VAL_ASSERT(is_block_pos_local(pos), "[ChunkData] Invalid local block position");
        return m_lighting_data[index(pos)];
    }

    [[nodiscard]] int block_count() const
    {
        return m_block_count;
    }

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(m_pos, m_block_data, m_lighting_data, m_block_count);
    }

private:
    static size_t index(const mve::Vector3i pos)
    {
        return pos.x + pos.y * sc_chunk_size + pos.z * sc_chunk_size * sc_chunk_size;
    }

    static size_t index2(const mve::Vector2i pos)
    {
        return pos.x + pos.y * sc_chunk_size;
    }

    static mve::Vector3i pos(const int index)
    {
        mve::Vector3i vector;
        vector.x = index % sc_chunk_size;
        vector.y = index / sc_chunk_size % sc_chunk_size;
        vector.z = index / (sc_chunk_size * sc_chunk_size);
        return vector;
    }

    static mve::Vector2i pos2(const int index)
    {
        mve::Vector2i vector;
        vector.x = index % sc_chunk_size;
        vector.y = index / sc_chunk_size % sc_chunk_size;
        return vector;
    }

    static constexpr int sc_chunk_size = 16;
    mve::Vector3i m_pos;
    std::array<uint8_t, sc_chunk_size * sc_chunk_size * sc_chunk_size> m_block_data = { 0 };
    std::array<uint8_t, sc_chunk_size * sc_chunk_size * sc_chunk_size> m_lighting_data = { 0 };
    int m_block_count = 0;
};