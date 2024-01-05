#pragma once

#include <functional>
#include <optional>
#include <set>
#include <unordered_map>

#include <mve/math/math.hpp>

#include "chunk_column.hpp"
#include "chunk_data.hpp"
#include "save_file.hpp"

class WorldGenerator;
class WorldData {
public:
    WorldData();

    ~WorldData();

    void create_or_load_chunk(mve::Vector2i chunk_pos);

    [[nodiscard]] std::optional<uint8_t> block_at(const mve::Vector3i block_pos) const
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        if (chunk_pos.z < -10 || chunk_pos.z >= 10) {
            return {};
        }
        if (!m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y })) {
            return {};
        }
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).get_block(block_pos);
    }

    void set_lighting(const mve::Vector3i pos, const uint8_t val)
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(pos);
        VV_DEB_ASSERT(m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y }), "[WorldData] Invalid chunk");
        m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).set_lighting(pos, val);
    }

    [[nodiscard]] std::optional<uint8_t> lighting_at(const mve::Vector3i block_pos) const
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        if (chunk_pos.z < -10 || chunk_pos.z >= 10) {
            return {};
        }
        if (!m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y })) {
            return {};
        }
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).lighting_at(block_pos);
    }

    [[nodiscard]] uint8_t block_at_local(mve::Vector3i chunk_pos, const mve::Vector3i block_pos) const
    {
        VV_DEB_ASSERT(contains_chunk(chunk_pos), "[WorldData] Invalid chunk")
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).get_block(block_pos);
    }

    [[nodiscard]] uint8_t lighting_at_local(mve::Vector3i chunk_pos, const mve::Vector3i block_pos) const
    {
        VV_DEB_ASSERT(contains_chunk(chunk_pos), "[WorldData] Invalid chunk")
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).lighting_at(block_pos);
    }

    [[nodiscard]] std::optional<uint8_t> block_at_relative(
        const mve::Vector3i chunk_pos, const mve::Vector3i local_block_pos) const
    {
        if (is_block_pos_local(local_block_pos)) {
            return block_at_local(chunk_pos, local_block_pos);
        }
        return block_at(block_local_to_world(chunk_pos, local_block_pos));
    }

    [[nodiscard]] std::optional<uint8_t> lighting_at_relative(
        const mve::Vector3i chunk_pos, const mve::Vector3i local_block_pos) const
    {
        if (is_block_pos_local(local_block_pos)) {
            return lighting_at_local(chunk_pos, local_block_pos);
        }
        return lighting_at(block_local_to_world(chunk_pos, local_block_pos));
    }

    void set_block(const mve::Vector3i block_pos, const uint8_t type)
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        VV_DEB_ASSERT(m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y }), "[WorldData] Invalid chunk");
        m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).set_block(block_pos, type);
        queue_save_chunk({ chunk_pos.x, chunk_pos.y });
    }

    void set_block_local(mve::Vector3i chunk_pos, const mve::Vector3i block_pos, const uint8_t type)
    {
        VV_DEB_ASSERT(m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y }), "[WorldData] Invalid chunk");
        m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).set_block(block_local_to_world(chunk_pos, block_pos), type);
        queue_save_chunk({ chunk_pos.x, chunk_pos.y });
    }

    ChunkColumn& chunk_column_data_at(const mve::Vector2i chunk_pos)
    {
        VV_DEB_ASSERT(m_chunk_columns.contains(chunk_pos), "[WorldData] Invalid chunk");
        return m_chunk_columns.at(chunk_pos);
    }

    [[nodiscard]] const ChunkColumn& chunk_column_data_at(const mve::Vector2i chunk_pos) const
    {
        VV_DEB_ASSERT(m_chunk_columns.contains(chunk_pos), "[WorldData] Invalid chunk");
        return m_chunk_columns.at(chunk_pos);
    }

    [[nodiscard]] const ChunkData& chunk_data_at(mve::Vector3i chunk_pos) const
    {
        VV_DEB_ASSERT(m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y }), "[WorldData] Invalid chunk");
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).chunk_data_at(chunk_pos);
    }

    ChunkData& chunk_data_at(mve::Vector3i chunk_pos)
    {
        VV_DEB_ASSERT(m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y }), "[WorldData] Invalid chunk");
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).chunk_data_at(chunk_pos);
    }

    [[nodiscard]] bool contains_chunk(mve::Vector3i chunk_pos) const
    {
        return chunk_pos.z >= -10 && chunk_pos.z < 10 && m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y });
    }

    [[nodiscard]] bool contains_column(const mve::Vector2i col_pos) const
    {
        return m_chunk_columns.contains(col_pos);
    }

    bool try_load_chunk_column_from_save(mve::Vector2i chunk_pos);

    void queue_save_chunk(mve::Vector2i pos);

    void set_player_chunk(mve::Vector2i chunk_pos);

    std::optional<mve::Vector2i> try_cull_chunk(float distance);

    [[nodiscard]] size_t chunk_count() const
    {
        return m_chunk_columns.size();
    }

private:
    void create_chunk_column(mve::Vector2i chunk_pos);

    void process_save_queue();

    void remove_chunk_column(mve::Vector2i chunk_pos);

    void sort_chunks();

    std::set<mve::Vector2i> m_save_queue;
    SaveFile m_save;
    mve::Vector2i m_player_chunk;
    std::unordered_map<mve::Vector2i, ChunkColumn> m_chunk_columns {};
    std::vector<mve::Vector2i> m_sorted_chunks {};

    std::function<bool(mve::Vector2i, mve::Vector2i)> compare_from_player
        = [&](const mve::Vector2i a, const mve::Vector2i b) {
              return distance_sqrd(mve::Vector2(a), mve::Vector2(m_player_chunk))
                  < distance_sqrd(mve::Vector2(b), mve::Vector2(m_player_chunk));
          };
};