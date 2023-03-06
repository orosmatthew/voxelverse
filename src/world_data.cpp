#include "world_data.hpp"

#include <cereal/archives/portable_binary.hpp>

WorldData::WorldData()
    : m_save(16 * 1024 * 1024, "world_data")
{
}

void WorldData::queue_save_chunk(mve::Vector3i pos)
{
    m_save_queue.insert(pos);
    if (m_save_queue.size() > 50) {
        process_save_queue();
    }
}

WorldData::~WorldData()
{
    process_save_queue();
}
void WorldData::process_save_queue()
{
    m_save.begin_batch();
    for (mve::Vector3i pos : m_save_queue) {
        m_save.insert<mve::Vector3i, ChunkData>(pos, m_chunks.at(pos));
    }
    m_save.submit_batch();
    m_save_queue.clear();
}
bool WorldData::try_load_chunk_from_save(mve::Vector3i chunk_pos)
{
    std::optional<ChunkData> chunk_data = m_save.at<mve::Vector3i, ChunkData>(chunk_pos);
    if (!chunk_data.has_value()) {
        return false;
    }
    chunk_data->set_modified_callback(m_modified_callback);
    m_chunks.insert({ chunk_pos, std::move(*chunk_data) });
    return true;
}
