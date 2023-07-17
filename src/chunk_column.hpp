#pragma once

#include "chunk_data.hpp"

#include <cereal/types/array.hpp>

#include <array>
#include <functional>
#include <optional>

class ChunkColumn {
public:
    inline ChunkColumn()
    {
        for (ChunkData& chunk : m_chunks) {
            chunk.set_modified_callback(m_chunk_modified_callback);
        }
    };

    inline ChunkColumn(mve::Vector2i chunk_pos)
        : m_pos(chunk_pos)
    {
        for (ChunkData& chunk : m_chunks) {
            chunk.set_modified_callback(m_chunk_modified_callback);
        }
    }

    inline void set_modified_callback(const std::function<void(mve::Vector2i, const ChunkColumn& column)>& callback)
    {
        m_modified_callback = std::move(callback);
    }

    inline void clear_modified_callback()
    {
        m_modified_callback.reset();
    }

    inline uint8_t get_block(mve::Vector3i block_pos) const
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        return m_chunks[chunk_pos.z + 10].get_block(block_world_to_local(block_pos));
    }

    inline void set_lighting(mve::Vector3i block_pos, uint8_t val)
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        m_chunks[chunk_pos.z + 10].set_lighting(block_world_to_local(block_pos), val);
    }

    inline uint8_t lighting_at(mve::Vector3i block_pos) const
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        return m_chunks[chunk_pos.z + 10].lighting_at(block_world_to_local(block_pos));
    }

    inline void set_block(mve::Vector3i block_pos, uint8_t type)
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        m_chunks.at(chunk_pos.z + 10).set_block(block_world_to_local(block_pos), type);
    }

    inline const ChunkData& chunk_data_at(mve::Vector3i chunk_pos) const
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
        archive(m_pos, m_chunks, m_generated);
    }

    inline ~ChunkColumn()
    {
        for (ChunkData& data : m_chunks) {
            data.remove_modified_callback();
        }
    }

    inline void set_generated(bool val)
    {
        m_generated = val;
    }

    inline bool is_generated() {
        return m_generated;
    }

private:
    std::function<void(const mve::Vector3i&, const ChunkData&)> m_chunk_modified_callback
        = [this](const mve::Vector3i& chunk_pos, const ChunkData&) {
              if (m_modified_callback.has_value()) {
                  std::invoke(m_modified_callback.value(), m_pos, *this);
              }
          };

    bool m_generated = false;
    mve::Vector2i m_pos;
    std::array<ChunkData, 20> m_chunks = {};
    std::optional<std::function<void(mve::Vector2i, const ChunkColumn& column)>> m_modified_callback {};
};