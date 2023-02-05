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

    void set_block(mve::Vector3i block_pos, uint8_t type);

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

private:
    static mve::Vector3i block_world_to_local(mve::Vector3i world_block_pos);

    std::unordered_map<mve::Vector3i, ChunkData> m_chunks {};
};