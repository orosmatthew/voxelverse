#pragma once

#include <array>

// ReSharper disable once CppUnusedIncludeDirective
#include <cereal/types/array.hpp>

#include "chunk_data.hpp"

class ChunkColumn {
public:
    enum GenLevel { none, terrain, trees, generated };

    ChunkColumn() = default;

    explicit ChunkColumn(const mve::Vector2i chunk_pos)
        : m_pos(chunk_pos)
    {
    }

    [[nodiscard]] uint8_t get_block(const mve::Vector3i block_pos) const
    {
        const mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        MVE_VAL_ASSERT(chunk_pos.z >= -10 && chunk_pos.z < 10, "[ChunkColumn] Invalid block position");
        return m_chunks[chunk_pos.z + 10].get_block(block_world_to_local(block_pos));
    }

    void set_lighting(const mve::Vector3i block_pos, const uint8_t val)
    {
        const mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        MVE_VAL_ASSERT(chunk_pos.z >= -10 && chunk_pos.z < 10, "[ChunkColumn] Invalid block position");
        m_chunks[chunk_pos.z + 10].set_lighting(block_world_to_local(block_pos), val);
    }

    [[nodiscard]] uint8_t lighting_at(const mve::Vector3i block_pos) const
    {
        const mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        MVE_VAL_ASSERT(chunk_pos.z >= -10 && chunk_pos.z < 10, "[ChunkColumn] Invalid block position");
        return m_chunks[chunk_pos.z + 10].lighting_at(block_world_to_local(block_pos));
    }

    void set_block(const mve::Vector3i block_pos, const uint8_t type)
    {
        const mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        MVE_VAL_ASSERT(chunk_pos.z >= -10 && chunk_pos.z < 10, "[ChunkColumn] Invalid block position");
        m_chunks[chunk_pos.z + 10].set_block(block_world_to_local(block_pos), type);
    }

    [[nodiscard]] const ChunkData& chunk_data_at(const mve::Vector3i chunk_pos) const
    {
        MVE_VAL_ASSERT(
            chunk_pos.x == m_pos.x && chunk_pos.y == m_pos.y && chunk_pos.z >= -10 && chunk_pos.z < 10,
            "[ChunkColumn] Invalid chunk position");
        return m_chunks[chunk_pos.z + 10];
    }

    ChunkData& chunk_data_at(const mve::Vector3i chunk_pos)
    {
        MVE_VAL_ASSERT(
            chunk_pos.x == m_pos.x && chunk_pos.y == m_pos.y && chunk_pos.z >= -10 && chunk_pos.z < 10,
            "[ChunkColumn] Invalid chunk position");
        return m_chunks[chunk_pos.z + 10];
    }

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(m_pos, m_chunks, m_gen_level);
    }

    void set_gen_level(const GenLevel level)
    {
        m_gen_level = level;
    }

    [[nodiscard]] GenLevel gen_level() const
    {
        return m_gen_level;
    }

    [[nodiscard]] mve::Vector2i pos() const
    {
        return m_pos;
    }

private:
    GenLevel m_gen_level = none;
    mve::Vector2i m_pos;
    std::array<ChunkData, 20> m_chunks = {};
};