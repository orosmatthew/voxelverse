#pragma once

#include <functional>
#include <optional>
#include <set>
#include <unordered_map>

#include "common.hpp"

#include <nnm/nnm.hpp>

#include "chunk_column.hpp"
#include "chunk_data.hpp"
#include "save_file.hpp"

class WorldGenerator;
class WorldData {
public:
    WorldData();

    ~WorldData();

    void create_or_load_chunk(nnm::Vector2i chunk_pos);

    [[nodiscard]] std::optional<uint8_t> block_at(const nnm::Vector3i block_pos) const
    {
        nnm::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        if (chunk_pos.z < -10 || chunk_pos.z >= 10) {
            return {};
        }
        if (!m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y })) {
            return {};
        }
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).get_block(block_pos);
    }

    void set_lighting(const nnm::Vector3i pos, const uint8_t val)
    {
        nnm::Vector3i chunk_pos = chunk_pos_from_block_pos(pos);
        VV_DEB_ASSERT(m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y }), "[WorldData] Invalid chunk");
        m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).set_lighting(pos, val);
    }

    [[nodiscard]] std::optional<uint8_t> lighting_at(const nnm::Vector3i block_pos) const
    {
        nnm::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        if (chunk_pos.z < -10 || chunk_pos.z >= 10) {
            return {};
        }
        if (!m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y })) {
            return {};
        }
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).lighting_at(block_pos);
    }

    [[nodiscard]] uint8_t block_at_local(nnm::Vector3i chunk_pos, const nnm::Vector3i block_pos) const
    {
        VV_DEB_ASSERT(contains_chunk(chunk_pos), "[WorldData] Invalid chunk")
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).get_block(block_pos);
    }

    [[nodiscard]] uint8_t lighting_at_local(nnm::Vector3i chunk_pos, const nnm::Vector3i block_pos) const
    {
        VV_DEB_ASSERT(contains_chunk(chunk_pos), "[WorldData] Invalid chunk")
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).lighting_at(block_pos);
    }

    [[nodiscard]] std::optional<uint8_t> block_at_relative(
        const nnm::Vector3i chunk_pos, const nnm::Vector3i local_block_pos) const
    {
        if (is_block_pos_local(local_block_pos)) {
            return block_at_local(chunk_pos, local_block_pos);
        }
        return block_at(block_local_to_world(chunk_pos, local_block_pos));
    }

    [[nodiscard]] std::optional<uint8_t> lighting_at_relative(
        const nnm::Vector3i chunk_pos, const nnm::Vector3i local_block_pos) const
    {
        if (is_block_pos_local(local_block_pos)) {
            return lighting_at_local(chunk_pos, local_block_pos);
        }
        return lighting_at(block_local_to_world(chunk_pos, local_block_pos));
    }

    void set_block(const nnm::Vector3i block_pos, const uint8_t type)
    {
        nnm::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        VV_DEB_ASSERT(m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y }), "[WorldData] Invalid chunk");
        m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).set_block(block_pos, type);
        queue_save_chunk({ chunk_pos.x, chunk_pos.y });
    }

    void set_block_local(nnm::Vector3i chunk_pos, const nnm::Vector3i block_pos, const uint8_t type)
    {
        VV_DEB_ASSERT(m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y }), "[WorldData] Invalid chunk");
        m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).set_block(block_local_to_world(chunk_pos, block_pos), type);
        queue_save_chunk({ chunk_pos.x, chunk_pos.y });
    }

    ChunkColumn& chunk_column_data_at(const nnm::Vector2i chunk_pos)
    {
        VV_DEB_ASSERT(m_chunk_columns.contains(chunk_pos), "[WorldData] Invalid chunk");
        return m_chunk_columns.at(chunk_pos);
    }

    [[nodiscard]] const ChunkColumn& chunk_column_data_at(const nnm::Vector2i chunk_pos) const
    {
        VV_DEB_ASSERT(m_chunk_columns.contains(chunk_pos), "[WorldData] Invalid chunk");
        return m_chunk_columns.at(chunk_pos);
    }

    [[nodiscard]] const ChunkData& chunk_data_at(nnm::Vector3i chunk_pos) const
    {
        VV_DEB_ASSERT(m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y }), "[WorldData] Invalid chunk");
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).chunk_data_at(chunk_pos);
    }

    ChunkData& chunk_data_at(nnm::Vector3i chunk_pos)
    {
        VV_DEB_ASSERT(m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y }), "[WorldData] Invalid chunk");
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).chunk_data_at(chunk_pos);
    }

    [[nodiscard]] bool contains_chunk(nnm::Vector3i chunk_pos) const
    {
        return chunk_pos.z >= -10 && chunk_pos.z < 10 && m_chunk_columns.contains({ chunk_pos.x, chunk_pos.y });
    }

    [[nodiscard]] bool contains_column(const nnm::Vector2i col_pos) const
    {
        return m_chunk_columns.contains(col_pos);
    }

    bool try_load_chunk_column_from_save(nnm::Vector2i chunk_pos);

    void queue_save_chunk(nnm::Vector2i pos);

    void set_player_chunk(nnm::Vector2i chunk_pos);

    std::optional<nnm::Vector2i> try_cull_chunk(float distance);

    [[nodiscard]] size_t chunk_count() const
    {
        return m_chunk_columns.size();
    }

private:
    void create_chunk_column(nnm::Vector2i chunk_pos);

    void process_save_queue();

    void remove_chunk_column(nnm::Vector2i chunk_pos);

    void sort_chunks();

    std::set<nnm::Vector2i> m_save_queue;
    SaveFile m_save;
    nnm::Vector2i m_player_chunk;
    std::unordered_map<nnm::Vector2i, ChunkColumn> m_chunk_columns {};
    std::vector<nnm::Vector2i> m_sorted_chunks {};

    std::function<bool(nnm::Vector2i, nnm::Vector2i)> compare_from_player
        = [&](const nnm::Vector2i a, const nnm::Vector2i b) {
              return nnm::Vector2f(a).distance_sqrd(nnm::Vector2f(m_player_chunk))
                  < nnm::Vector2f(b).distance_sqrd(nnm::Vector2f(m_player_chunk));
          };
};