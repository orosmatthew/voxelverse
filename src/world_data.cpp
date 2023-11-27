#include "world_data.hpp"

#include <ranges>

WorldData::WorldData()
    : m_save(16 * 1024 * 1024, "world_data")
    , m_player_chunk(mve::Vector2i(0, 0))
{
}

void WorldData::queue_save_chunk(const mve::Vector2i pos)
{
    m_save_queue.insert(pos);
    if (m_save_queue.size() > 50) {
        process_save_queue();
    }
}
void WorldData::set_player_chunk(const mve::Vector2i chunk_pos)
{
    const mve::Vector2i prev = m_player_chunk;
    m_player_chunk = chunk_pos;
    if (m_player_chunk != prev) {
        sort_chunks();
    }
}
std::optional<mve::Vector2i> WorldData::try_cull_chunk(const float distance)
{
    if (mve::Vector2i furthest_chunk = m_sorted_chunks[m_sorted_chunks.size() - 1];
        mve::distance(mve::Vector2(furthest_chunk), mve::Vector2(m_player_chunk)) > distance) {
        if (m_save_queue.contains(furthest_chunk)) {
            process_save_queue();
        }
        m_chunk_columns.erase(furthest_chunk);
        m_sorted_chunks.pop_back();
        return furthest_chunk;
    }
    return {};
}

WorldData::~WorldData()
{
    for (auto& col : m_chunk_columns | std::views::values) {
        queue_save_chunk(col.pos());
    }
    process_save_queue();
}

void WorldData::create_or_load_chunk(const mve::Vector2i chunk_pos)
{
    if (!try_load_chunk_column_from_save(chunk_pos)) {
        create_chunk_column(chunk_pos);
    }
}

void WorldData::create_chunk_column(mve::Vector2i chunk_pos)
{
    if (auto [_, inserted] = m_chunk_columns.insert({ chunk_pos, ChunkColumn(chunk_pos) }); inserted) {
        insert_sorted(m_sorted_chunks, chunk_pos, compare_from_player);
    }
}

void WorldData::process_save_queue()
{
    m_save.begin_batch();
    for (mve::Vector2i pos : m_save_queue) {
        if (m_chunk_columns.contains(pos)) {
            m_save.insert<mve::Vector2i, ChunkColumn>(pos, m_chunk_columns.at(pos));
        }
    }
    m_save.submit_batch();
    m_save_queue.clear();
}

void WorldData::remove_chunk_column(const mve::Vector2i chunk_pos)
{
    if (m_save_queue.contains(chunk_pos)) {
        process_save_queue();
    }
    m_chunk_columns.erase(chunk_pos);
    std::erase(m_sorted_chunks, chunk_pos);
}

void WorldData::sort_chunks()
{
    std::ranges::sort(m_sorted_chunks, compare_from_player);
}

bool WorldData::try_load_chunk_column_from_save(mve::Vector2i chunk_pos)
{
    std::optional<ChunkColumn> data = m_save.at<mve::Vector2i, ChunkColumn>(chunk_pos);
    if (!data.has_value()) {
        return false;
    }
    if (auto [_, inserted] = m_chunk_columns.insert({ chunk_pos, *data }); inserted) {
        insert_sorted(m_sorted_chunks, chunk_pos, compare_from_player);
    }
    return true;
}