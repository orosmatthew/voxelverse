#pragma once

#include <array>

// ReSharper disable once CppUnusedIncludeDirective
#include <cereal/types/array.hpp>

#include "common.hpp"

#include <nnm/nnm.hpp>

#include "chunk_data.hpp"

class ChunkColumn {
public:
    enum GenLevel { none, terrain, trees, generated };

    ChunkColumn() = default;

    explicit ChunkColumn(const nnm::Vector2i chunk_pos)
        : m_pos(chunk_pos)
    {
    }

    [[nodiscard]] uint8_t get_block(const nnm::Vector3i block_pos) const
    {
        const nnm::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        VV_DEB_ASSERT(chunk_pos.z >= -10 && chunk_pos.z < 10, "[ChunkColumn] Invalid block position");
        return m_chunks[chunk_pos.z + 10].get_block(block_world_to_local(block_pos));
    }

    void set_lighting(const nnm::Vector3i block_pos, const uint8_t val)
    {
        const nnm::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        VV_DEB_ASSERT(chunk_pos.z >= -10 && chunk_pos.z < 10, "[ChunkColumn] Invalid block position");
        m_chunks[chunk_pos.z + 10].set_lighting(block_world_to_local(block_pos), val);
    }

    [[nodiscard]] uint8_t lighting_at(const nnm::Vector3i block_pos) const
    {
        const nnm::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        VV_DEB_ASSERT(chunk_pos.z >= -10 && chunk_pos.z < 10, "[ChunkColumn] Invalid block position");
        return m_chunks[chunk_pos.z + 10].lighting_at(block_world_to_local(block_pos));
    }

    void set_block(const nnm::Vector3i block_pos, const uint8_t type)
    {
        const nnm::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        VV_DEB_ASSERT(chunk_pos.z >= -10 && chunk_pos.z < 10, "[ChunkColumn] Invalid block position");
        m_chunks[chunk_pos.z + 10].set_block(block_world_to_local(block_pos), type);
    }

    [[nodiscard]] const ChunkData& chunk_data_at(const nnm::Vector3i chunk_pos) const
    {
        VV_DEB_ASSERT(
            chunk_pos.x == m_pos.x && chunk_pos.y == m_pos.y && chunk_pos.z >= -10 && chunk_pos.z < 10,
            "[ChunkColumn] Invalid chunk position");
        return m_chunks[chunk_pos.z + 10];
    }

    ChunkData& chunk_data_at(const nnm::Vector3i chunk_pos)
    {
        VV_DEB_ASSERT(
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

    [[nodiscard]] nnm::Vector2i pos() const
    {
        return m_pos;
    }

private:
    GenLevel m_gen_level = none;
    nnm::Vector2i m_pos;
    std::array<ChunkData, 20> m_chunks = {};
};