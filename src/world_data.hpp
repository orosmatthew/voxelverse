#pragma once

#include <functional>
#include <optional>
#include <unordered_map>

#include "chunk_data.hpp"
#include "math/math.hpp"
#include "world_generator.hpp"

class WorldData {
public:
    WorldData();

    void generate(const WorldGenerator& generator, mve::Vector3i from, mve::Vector3i to);

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
    }

    inline void set_block_local(mve::Vector3i chunk_pos, mve::Vector3i block_pos, uint8_t type)
    {
        m_chunks.at(chunk_pos).set_block(block_world_to_local(block_pos), type);
    }

    void for_all_chunk_data(const std::function<void(mve::Vector3i chunk_pos, const ChunkData& chunk_data)>& func) const
    {
        for (const auto& [pos, data] : m_chunks) {
            std::invoke(func, pos, data);
        }
    }

    inline const ChunkData& chunk_data_at(mve::Vector3i chunk_pos) const
    {
        return m_chunks.at(chunk_pos);
    }

    inline bool chunk_in_bounds(mve::Vector3i chunk_pos) const
    {
        return m_chunks.contains(chunk_pos);
    }

    static inline mve::Vector3i chunk_pos_from_block_pos(mve::Vector3i block_pos)
    {
        return { static_cast<int>(mve::floor(block_pos.x / 16.0f)),
                 static_cast<int>(mve::floor(block_pos.y / 16.0f)),
                 static_cast<int>(mve::floor(block_pos.z / 16.0f)) };
    }

    static inline mve::Vector3i block_local_to_world(mve::Vector3i chunk_pos, mve::Vector3i local_block_pos)
    {
        return { chunk_pos.x * 16 + local_block_pos.x,
                 chunk_pos.y * 16 + local_block_pos.y,
                 chunk_pos.z * 16 + local_block_pos.z };
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

    static inline bool is_block_pos_local(mve::Vector3i block_pos)
    {
        return block_pos.x >= 0 && block_pos.x < 16 && block_pos.y >= 0 && block_pos.y < 16 && block_pos.z >= 0
            && block_pos.z < 16;
    }

private:
    std::unordered_map<mve::Vector3i, ChunkData> m_chunks {};
};