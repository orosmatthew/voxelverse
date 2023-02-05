#include "world_data.hpp"

#include "math/functions.hpp"

WorldData::WorldData(const WorldGenerator& generator, mve::Vector3i from, mve::Vector3i to)
{
    for (int x = from.x; x < to.x; x++) {
        for (int y = from.y; y < to.y; y++) {
            for (int z = from.z; z < to.z; z++) {
                ChunkData chunk_data({ x, y, z });
                chunk_data.generate(generator);
                m_chunks.insert({ { x, y, z }, std::move(chunk_data) });
            }
        }
    }
}

std::optional<uint8_t> WorldData::block_at(mve::Vector3i block_pos) const
{
    mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
    if (!m_chunks.contains(chunk_pos)) {
        return {};
    }
    return m_chunks.at(chunk_pos).get_block(block_world_to_local(block_pos));
}
mve::Vector3i WorldData::block_world_to_local(mve::Vector3i world_block_pos)
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

void WorldData::for_all_chunk_data(
    const std::function<void(mve::Vector3i chunk_pos, const ChunkData& chunk_data)>& func) const
{
    for (const auto& [pos, data] : m_chunks) {
        std::invoke(func, pos, data);
    }
}
void WorldData::set_block(mve::Vector3i block_pos, uint8_t type)
{
    mve::Vector3i chunk_pos = chunk_pos_from_block_pos(block_pos);
    m_chunks.at(chunk_pos).set_block(block_world_to_local(block_pos), type);
}
mve::Vector3i WorldData::chunk_pos_from_block_pos(mve::Vector3i block_pos)
{
    return { static_cast<int>(mve::floor(block_pos.x / 16.0f)),
             static_cast<int>(mve::floor(block_pos.y / 16.0f)),
             static_cast<int>(mve::floor(block_pos.z / 16.0f)) };
}
const ChunkData& WorldData::chunk_data_at(mve::Vector3i chunk_pos) const
{
    return m_chunks.at(chunk_pos);
}
bool WorldData::chunk_in_bounds(mve::Vector3i chunk_pos) const
{
    return m_chunks.contains(chunk_pos);
}
