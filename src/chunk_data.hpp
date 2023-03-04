#pragma once

#include <array>
#include <functional>
#include <optional>
#include <stdint.h>

#include <cereal/types/array.hpp>

#include "common.hpp"
#include "mve/math/math.hpp"

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
    explicit ChunkData(mve::Vector3i chunk_pos);

    inline void set_modified_callback(std::function<void(mve::Vector3i, const ChunkData&)> func)
    {
        m_modified_callback = func;
    }

    inline void remove_modified_callback()
    {
        m_modified_callback.reset();
    }

    mve::Vector3i position() const;

    void set_block(mve::Vector3i pos, uint8_t type);
    inline uint8_t get_block(mve::Vector3i pos) const
    {
        return m_block_data[index(pos)];
    }

    inline int block_count() const
    {
        return m_block_count;
    }

    bool in_bounds(mve::Vector3i pos) const;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(m_pos, m_block_data, m_block_count);
    }

private:
    static inline size_t index(mve::Vector3i pos)
    {
        return pos.x + pos.y * sc_chunk_size + pos.z * sc_chunk_size * sc_chunk_size;
    }

    static inline mve::Vector3i pos(size_t index)
    {
        mve::Vector3i vector;
        vector.x = index % sc_chunk_size;
        vector.y = (index / sc_chunk_size) % sc_chunk_size;
        vector.z = index / (sc_chunk_size * sc_chunk_size);
        return vector;
    }

    static const int sc_chunk_size = 16;
    mve::Vector3i m_pos;
    std::array<uint8_t, sc_chunk_size* sc_chunk_size* sc_chunk_size> m_block_data = { 0 };
    int m_block_count = 0;
    std::optional<std::function<void(mve::Vector3i, const ChunkData&)>> m_modified_callback;
};