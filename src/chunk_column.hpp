#pragma once

#include "chunk_data.hpp"

#include <cereal/types/array.hpp>

#include <array>
#include <functional>
#include <optional>

class ChunkColumn {
public:
    enum GenLevel { none, terrain, trees, generated };

    inline ChunkColumn() = default;

    inline explicit ChunkColumn(mve::Vector2i chunk_pos)
        : m_pos(chunk_pos)
    {
    }

    [[nodiscard]] inline uint8_t get_block(mve::Vector3i block_pos) const
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
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

    inline void set_block(mve::Vector3i block_pos, uint8_t type)
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        m_chunks.at(chunk_pos.z + 10).set_block(block_world_to_local(block_pos), type);
    }

    [[nodiscard]] inline const ChunkData& chunk_data_at(mve::Vector3i chunk_pos) const
    {
        return m_chunks[chunk_pos.z + 10];
    }

    inline ChunkData& chunk_data_at(mve::Vector3i chunk_pos)
    {
        return m_chunks[chunk_pos.z + 10];
    }

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(m_pos, m_chunks, m_gen_level);
    }

    inline void set_gen_level(GenLevel level)
    {
        m_gen_level = level;
    }

    [[nodiscard]] inline GenLevel gen_level() const
    {
        return m_gen_level;
    }

private:
    GenLevel m_gen_level = GenLevel::none;
    mve::Vector2i m_pos;
    std::array<ChunkData, 20> m_chunks = {};
};