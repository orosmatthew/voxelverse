#pragma once

#include <array>
#include <cstdint>

// ReSharper disable once CppUnusedIncludeDirective
#include <cereal/types/array.hpp>
// ReSharper disable once CppUnusedIncludeDirective
#include <cereal/types/vector.hpp>

#include "common.hpp"

#include <nnm/nnm.hpp>

#include "../common/assert.hpp"

inline nnm::Vector3i direction_vector(const Direction dir)
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

    explicit ChunkData(nnm::Vector3i chunk_pos);

    void reset_lighting(const uint8_t value = 0)
    {
        std::ranges::fill(m_lighting_data, value);
    }

    [[nodiscard]] nnm::Vector3i position() const
    {
        return m_pos;
    }

    void set_block(nnm::Vector3i pos, uint8_t type);
    [[nodiscard]] uint8_t get_block(const nnm::Vector3i pos) const
    {
        VV_DEB_ASSERT(is_block_pos_local(pos), "[ChunkData] Invalid local block position");
        return m_block_data[index(pos)];
    }

    void set_lighting(const nnm::Vector3i pos, const uint8_t val)
    {
        VV_DEB_ASSERT(is_block_pos_local(pos), "[ChunkData] Invalid local block position");
        VV_DEB_ASSERT(val <= 15, "[ChunkData] Lighting is not between 0 and 15")
        m_lighting_data[index(pos)] = val;
    }

    [[nodiscard]] uint8_t lighting_at(const nnm::Vector3i pos) const
    {
        VV_DEB_ASSERT(is_block_pos_local(pos), "[ChunkData] Invalid local block position");
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
    static size_t index(const nnm::Vector3i pos)
    {
        return pos.x + pos.y * sc_chunk_size + pos.z * sc_chunk_size * sc_chunk_size;
    }

    static size_t index2(const nnm::Vector2i pos)
    {
        return pos.x + pos.y * sc_chunk_size;
    }

    static nnm::Vector3i pos(const int index)
    {
        nnm::Vector3i vector;
        vector.x = index % sc_chunk_size;
        vector.y = index / sc_chunk_size % sc_chunk_size;
        vector.z = index / (sc_chunk_size * sc_chunk_size);
        return vector;
    }

    static nnm::Vector2i pos2(const int index)
    {
        nnm::Vector2i vector;
        vector.x = index % sc_chunk_size;
        vector.y = index / sc_chunk_size % sc_chunk_size;
        return vector;
    }

    static constexpr int sc_chunk_size = 16;
    nnm::Vector3i m_pos;
    std::array<uint8_t, sc_chunk_size * sc_chunk_size * sc_chunk_size> m_block_data = { 0 };
    std::array<uint8_t, sc_chunk_size * sc_chunk_size * sc_chunk_size> m_lighting_data = { 0 };
    int m_block_count = 0;
};