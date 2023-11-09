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
        return m_chunks[chunk_pos.z + 10].get_block(block_world_to_local(block_pos));
    }

    //    inline void set_lighting(mve::Vector3i block_pos, uint8_t val)
    //    {
    //        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
    //        m_chunks[chunk_pos.z + 10].set_lighting(block_world_to_local(block_pos), val);
    //    }

    //    inline uint8_t lighting_at(mve::Vector3i block_pos) const
    //    {
    //        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
    //        return m_chunks[chunk_pos.z + 10].lighting_at(block_world_to_local(block_pos));
    //    }

    void set_block(const mve::Vector3i block_pos, const uint8_t type)
    {
        const mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        m_chunks.at(chunk_pos.z + 10).set_block(block_world_to_local(block_pos), type);
    }

    [[nodiscard]] const ChunkData& chunk_data_at(const mve::Vector3i chunk_pos) const
    {
        return m_chunks[chunk_pos.z + 10];
    }

    ChunkData& chunk_data_at(const mve::Vector3i chunk_pos)
    {
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

private:
    GenLevel m_gen_level = none;
    mve::Vector2i m_pos;
    std::array<ChunkData, 20> m_chunks = {};
};