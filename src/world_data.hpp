#pragma once

#include <functional>
#include <optional>
#include <set>
#include <unordered_map>

#include "chunk_column.hpp"
#include "chunk_data.hpp"
#include "mve/math/math.hpp"
#include "save_file.hpp"

class WorldData {
public:
    WorldData();

    ~WorldData();

    void create_chunk_column(mve::Vector2i chunk_pos);

    [[nodiscard]] std::optional<uint8_t> block_at(const mve::Vector3i block_pos) const
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        if (chunk_pos.z < -10 || chunk_pos.z >= 10) {
            return {};
        }
        if (const auto result = m_chunk_columns.find({ chunk_pos.x, chunk_pos.y }); result == m_chunk_columns.end()) {
            return {};
        }
        else {
            return result->second.get_block(block_pos);
        }
    }

    //    inline void set_lighting(mve::Vector3i pos, uint8_t val)
    //    {
    //        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(pos);
    //        m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).set_lighting(pos, val);
    //    }

    //    inline std::optional<uint8_t> lighting_at(mve::Vector3i block_pos) const
    //    {
    //        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
    //        if (chunk_pos.z < -10 || chunk_pos.z >= 10) {
    //            return {};
    //        }
    //        auto result = m_chunk_columns.find({ chunk_pos.x, chunk_pos.y });
    //        if (result == m_chunk_columns.end()) {
    //            return {};
    //        }
    //        else {
    //            return result->second.lighting_at(block_pos);
    //        }
    //    }

    [[nodiscard]] uint8_t block_at_local(mve::Vector3i chunk_pos, const mve::Vector3i block_pos) const
    {
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).get_block(block_pos);
    }

    [[nodiscard]] std::optional<uint8_t> block_at_relative(
        const mve::Vector3i chunk_pos, const mve::Vector3i local_block_pos) const
    {
        if (is_block_pos_local(local_block_pos)) {
            return block_at_local(chunk_pos, local_block_pos);
        }
        return block_at(block_local_to_world(chunk_pos, local_block_pos));
    }

    void set_block(const mve::Vector3i block_pos, const uint8_t type)
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).set_block(block_pos, type);
        queue_save_chunk({ chunk_pos.x, chunk_pos.y });
    }

    void set_block_local(mve::Vector3i chunk_pos, const mve::Vector3i block_pos, const uint8_t type)
    {
        m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).set_block(block_local_to_world(chunk_pos, block_pos), type);
        queue_save_chunk({ chunk_pos.x, chunk_pos.y });
    }

    ChunkColumn& chunk_column_data_at(const mve::Vector2i chunk_pos)
    {
        return m_chunk_columns.at(chunk_pos);
    }

    [[nodiscard]] const ChunkData& chunk_data_at(mve::Vector3i chunk_pos) const
    {
        return m_chunk_columns.at({ chunk_pos.x, chunk_pos.y }).chunk_data_at(chunk_pos);
    }

    ChunkData& chunk_data_at(mve::Vector3i chunk_pos)
    {
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

    static mve::Vector3i chunk_pos_from_block_pos(const mve::Vector3i block_pos)
    {
        return { static_cast<int>(mve::floor(static_cast<float>(block_pos.x) / 16.0f)),
                 static_cast<int>(mve::floor(static_cast<float>(block_pos.y) / 16.0f)),
                 static_cast<int>(mve::floor(static_cast<float>(block_pos.z) / 16.0f)) };
    }

    static int chunk_height_from_block_height(const int block_height)
    {
        return static_cast<int>(mve::floor(static_cast<float>(block_height) / 16.0f));
    }

    static mve::Vector3i block_local_to_world(const mve::Vector3i chunk_pos, const mve::Vector3i local_block_pos)
    {
        return { chunk_pos.x * 16 + local_block_pos.x,
                 chunk_pos.y * 16 + local_block_pos.y,
                 chunk_pos.z * 16 + local_block_pos.z };
    }

    static mve::Vector2i block_local_to_world_col(const mve::Vector2i chunk_pos, const mve::Vector2i local_block_pos)
    {
        return { chunk_pos.x * 16 + local_block_pos.x, chunk_pos.y * 16 + local_block_pos.y };
    }

    static mve::Vector3i block_world_to_local(const mve::Vector3i world_block_pos)
    {
        mve::Vector3i mod = world_block_pos % 16;
        if (mod.x < 0) {
            mod.x = 16 + mod.x;
        }
        if (mod.y < 0) {
            mod.y = 16 + mod.y;
        }
        if (mod.z < 0) {
            mod.z = 16 + mod.z;
        }
        return mod;
    }

    static int block_height_world_to_local(const int world_block_height)
    {
        int mod = world_block_height % 16;
        if (mod < 0) {
            mod = 16 + mod;
        }
        return mod;
    }

    static bool is_block_pos_local(const mve::Vector3i block_pos)
    {
        return block_pos.x >= 0 && block_pos.x < 16 && block_pos.y >= 0 && block_pos.y < 16 && block_pos.z >= 0
            && block_pos.z < 16;
    }

    static bool is_block_pos_local_col(const mve::Vector2i block_pos)
    {
        return block_pos.x >= 0 && block_pos.x < 16 && block_pos.y >= 0 && block_pos.y < 16;
    }

    static bool is_block_height_world_valid(const int height)
    {
        return height >= -160 && height < 160;
    }

    void push_chunk_lighting_update(const mve::Vector3i chunk_pos)
    {
        if (std::ranges::find(m_chunk_lighting_update_list, chunk_pos) == m_chunk_lighting_update_list.end()) {
            m_chunk_lighting_update_list.push_back(chunk_pos);
        }
    }

    //    void process_chunk_lighting_updates();

    void queue_save_chunk(mve::Vector2i pos);

    void set_player_chunk(mve::Vector2i chunk_pos);

    void cull_chunks(float distance);

private:
    void process_save_queue();

    void remove_chunk_column(mve::Vector2i chunk_pos);

    void sort_chunks();
    //    void spread_light(const mve::Vector3i light_pos);

    //    void propagate_light(mve::Vector3i chunk_pos);

    std::set<mve::Vector2i> m_save_queue;
    SaveFile m_save;
    mve::Vector2i m_player_chunk;
    std::unordered_map<mve::Vector2i, ChunkColumn> m_chunk_columns {};
    std::vector<mve::Vector2i> m_sorted_chunks {};
    std::vector<mve::Vector3i> m_chunk_lighting_update_list {};
};