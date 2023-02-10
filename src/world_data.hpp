#pragma once

#include <functional>
#include <optional>
#include <unordered_map>

#include "chunk_data.hpp"
#include "math/math.hpp"
#include "world_generator.hpp"

class WorldData {
public:
    WorldData(const WorldGenerator& generator, mve::Vector3i from, mve::Vector3i to);

    std::optional<uint8_t> block_at(mve::Vector3i block_pos) const;

    uint8_t block_at_local(mve::Vector3i chunk_pos, mve::Vector3i block_pos) const;

    std::optional<uint8_t> block_at_relative(mve::Vector3i chunk_pos, mve::Vector3i local_block_pos) const;

    void set_block(mve::Vector3i block_pos, uint8_t type);

    void set_block_local(mve::Vector3i chunk_pos, mve::Vector3i block_pos, uint8_t type);

    void for_all_chunk_data(
        const std::function<void(mve::Vector3i chunk_pos, const ChunkData& chunk_data)>& func) const;

    const ChunkData& chunk_data_at(mve::Vector3i chunk_pos) const;

    bool chunk_in_bounds(mve::Vector3i chunk_pos) const;

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