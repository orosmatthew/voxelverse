#pragma once

#include <functional>
#include <optional>
#include <set>
#include <unordered_map>

#include "chunk_data.hpp"
#include "mve/math/math.hpp"
#include "save_file.hpp"

class WorldData {
public:
    WorldData();

    ~WorldData();

    inline void remove_chunk(mve::Vector3i chunk_pos)
    {
        if (m_save_queue.contains(chunk_pos)) {
            process_save_queue();
        }
        m_chunks.erase(chunk_pos);
    }

    inline void create_chunk(mve::Vector3i chunk_pos)
    {
        m_chunks.insert({ chunk_pos, ChunkData(chunk_pos) });
        m_chunks.at(chunk_pos).set_modified_callback(m_modified_callback);
        queue_save_chunk(chunk_pos);
    }

    inline std::optional<uint8_t> block_at(mve::Vector3i block_pos) const
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        auto result = m_chunks.find(chunk_pos);
        if (result == m_chunks.end()) {
            return {};
        }
        else {
            return result->second.get_block(block_world_to_local(block_pos));
        }
    }

    inline void set_lighting(mve::Vector3i pos, uint8_t val)
    {
        m_chunks.at(chunk_pos_from_block_pos(pos)).set_lighting(block_world_to_local(pos), val);
    }

    inline std::optional<uint8_t> lighting_at(mve::Vector3i block_pos) const
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        auto result = m_chunks.find(chunk_pos);
        if (result == m_chunks.end()) {
            return {};
        }
        else {
            return result->second.lighting_at(block_world_to_local(block_pos));
        }
    }

    inline uint8_t block_at_local(mve::Vector3i chunk_pos, mve::Vector3i block_pos) const
    {
        return m_chunks.at(chunk_pos).get_block(block_world_to_local(block_pos));
    }

    inline std::optional<uint8_t> block_at_relative(mve::Vector3i chunk_pos, mve::Vector3i local_block_pos) const
    {
        if (WorldData::is_block_pos_local(local_block_pos)) {
            return block_at_local(chunk_pos, local_block_pos);
        }
        else {
            return block_at(WorldData::block_local_to_world(chunk_pos, local_block_pos));
        }
    }

    inline void set_block(mve::Vector3i block_pos, uint8_t type)
    {
        mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
        m_chunks.at(chunk_pos).set_block(block_world_to_local(block_pos), type);
        queue_save_chunk(chunk_pos);
    }

    inline void set_block_local(mve::Vector3i chunk_pos, mve::Vector3i block_pos, uint8_t type)
    {
        m_chunks.at(chunk_pos).set_block(block_world_to_local(block_pos), type);
        queue_save_chunk(chunk_pos);
    }

    inline void for_all_chunk_data(
        const std::function<void(mve::Vector3i chunk_pos, const ChunkData& chunk_data)>& func) const
    {
        for (const auto& [pos, data] : m_chunks) {
            std::invoke(func, pos, data);
        }
    }

    inline const ChunkData& chunk_data_at(mve::Vector3i chunk_pos) const
    {
        return m_chunks.at(chunk_pos);
    }

    inline ChunkData& chunk_data_at(mve::Vector3i chunk_pos)
    {
        return m_chunks.at(chunk_pos);
    }

    inline bool contains_chunk(mve::Vector3i chunk_pos) const
    {
        return m_chunks.contains(chunk_pos);
    }

    bool try_load_chunk_from_save(mve::Vector3i chunk_pos);

    static inline mve::Vector3i chunk_pos_from_block_pos(mve::Vector3i block_pos)
    {
        return { static_cast<int>(mve::floor(block_pos.x / 16.0f)),
                 static_cast<int>(mve::floor(block_pos.y / 16.0f)),
                 static_cast<int>(mve::floor(block_pos.z / 16.0f)) };
    }

    static inline int chunk_height_from_block_height(int block_height)
    {
        return static_cast<int>(mve::floor(block_height / 16.0f));
    }

    static inline mve::Vector3i block_local_to_world(mve::Vector3i chunk_pos, mve::Vector3i local_block_pos)
    {
        return { chunk_pos.x * 16 + local_block_pos.x,
                 chunk_pos.y * 16 + local_block_pos.y,
                 chunk_pos.z * 16 + local_block_pos.z };
    }

    static inline mve::Vector2i block_local_to_world_col(mve::Vector2i chunk_pos, mve::Vector2i local_block_pos)
    {
        return { chunk_pos.x * 16 + local_block_pos.x, chunk_pos.y * 16 + local_block_pos.y };
    }

    static inline mve::Vector3i block_world_to_local(mve::Vector3i world_block_pos)
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

    static inline int block_height_world_to_local(int world_block_height)
    {
        int mod = world_block_height % 16;
        if (mod < 0) {
            mod = 16 + mod;
        }
        return mod;
    }

    static inline bool is_block_pos_local(mve::Vector3i block_pos)
    {
        return block_pos.x >= 0 && block_pos.x < 16 && block_pos.y >= 0 && block_pos.y < 16 && block_pos.z >= 0
            && block_pos.z < 16;
    }

    static inline bool is_block_pos_local_col(mve::Vector2i block_pos)
    {
        return block_pos.x >= 0 && block_pos.x < 16 && block_pos.y >= 0 && block_pos.y < 16;
    }

    static inline bool is_block_height_world_valid(int height)
    {
        return height >= -160 && height < 160;
    }

    inline void push_chunk_lighting_update(mve::Vector3i chunk_pos)
    {
        if (std::find(m_chunk_lighting_update_list.begin(), m_chunk_lighting_update_list.end(), chunk_pos)
            == m_chunk_lighting_update_list.end()) {
            m_chunk_lighting_update_list.push_back(chunk_pos);
        }
    }

    void process_chunk_lighting_updates();

private:
    void queue_save_chunk(mve::Vector3i pos);

    void process_save_queue();

    std::function<void(mve::Vector3i, const ChunkData&)> m_modified_callback
        = [this](mve::Vector3i chunk_pos, const ChunkData& chunk_data) { queue_save_chunk(chunk_pos); };

    void spread_light(const mve::Vector3i light_pos);

    void propagate_light(mve::Vector3i chunk_pos);

    std::set<mve::Vector3i> m_save_queue;
    SaveFile m_save;
    std::unordered_map<mve::Vector3i, ChunkData> m_chunks {};
    std::vector<mve::Vector3i> m_chunk_lighting_update_list {};
};