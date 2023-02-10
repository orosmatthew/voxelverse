#include "world_data.hpp"

#include "math/math.hpp"

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
    auto result = m_chunks.find(chunk_pos);
    if (result == m_chunks.end()) {
        return {};
    }
    else {
        return result->second.get_block(block_world_to_local(block_pos));
    }
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
const ChunkData& WorldData::chunk_data_at(mve::Vector3i chunk_pos) const
{
    return m_chunks.at(chunk_pos);
}
bool WorldData::chunk_in_bounds(mve::Vector3i chunk_pos) const
{
    return m_chunks.contains(chunk_pos);
}

void WorldData::set_block_local(mve::Vector3i chunk_pos, mve::Vector3i block_pos, uint8_t type)
{
    m_chunks.at(chunk_pos).set_block(block_world_to_local(block_pos), type);
}
uint8_t WorldData::block_at_local(mve::Vector3i chunk_pos, mve::Vector3i block_pos) const
{
    return m_chunks.at(chunk_pos).get_block(block_world_to_local(block_pos));
}
std::optional<uint8_t> WorldData::block_at_relative(mve::Vector3i chunk_pos, mve::Vector3i local_block_pos) const
{
    if (WorldData::is_block_pos_local(local_block_pos)) {
        return block_at_local(chunk_pos, local_block_pos);
    }
    else {
        return block_at(WorldData::block_local_to_world(chunk_pos, local_block_pos));
    }
}
