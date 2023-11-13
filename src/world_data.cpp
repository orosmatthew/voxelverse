#include "world_data.hpp"

WorldData::WorldData()
    : m_player_chunk(mve::Vector2i(0, 0))
    , m_save(16 * 1024 * 1024, "world_data")
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

void WorldData::cull_chunks(const float distance)
{
    for (size_t i = m_sorted_chunks.size() - 1; i > 0; i--) {
        if (mve::distance(mve::Vector2(m_sorted_chunks[i]), mve::Vector2(m_player_chunk)) > distance) {
            m_chunk_columns.erase(m_sorted_chunks[i]);
            m_sorted_chunks.pop_back();
        }
        else {
            break;
        }
    }
}

WorldData::~WorldData()
{
    process_save_queue();
}

void WorldData::create_chunk_column(mve::Vector2i chunk_pos)
{
    m_chunk_columns.insert({ chunk_pos, ChunkColumn(chunk_pos) });
    m_sorted_chunks.push_back(chunk_pos);
    sort_chunks();
    queue_save_chunk(chunk_pos);
}

void WorldData::process_save_queue()
{
    m_save.begin_batch();
    for (mve::Vector2i pos : m_save_queue) {
        if (m_chunk_columns.at(pos).gen_level() == ChunkColumn::GenLevel::generated) {
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
    std::ranges::sort(m_sorted_chunks, [&](const mve::Vector2i a, const mve::Vector2i b) {
        return distance_sqrd(mve::Vector2(a), mve::Vector2(m_player_chunk))
            < distance_sqrd(mve::Vector2(b), mve::Vector2(m_player_chunk));
    });
}

bool WorldData::try_load_chunk_column_from_save(mve::Vector2i chunk_pos)
{
    std::optional<ChunkColumn> data = m_save.at<mve::Vector2i, ChunkColumn>(chunk_pos);
    if (!data.has_value()) {
        return false;
    }
    m_chunk_columns.insert({ chunk_pos, std::move(*data) });
    m_sorted_chunks.push_back(chunk_pos);
    sort_chunks();
    return true;
}

// NOTE: Not thread safe
// void WorldData::spread_light(const mve::Vector3i light_pos)
//{
//    static std::vector<mve::Vector3i> prev_list;
//    static std::vector<mve::Vector3i> current_list;
//    static std::vector<mve::Vector3i> next_list;
//    prev_list.clear();
//    current_list.clear();
//    next_list.clear();
//
//    current_list.push_back(light_pos);
//
//    int light_val = 15;
//
//    while (!current_list.empty()) {
//        for (mve::Vector3i pos : current_list) {
//            for (int i = 0; i < 6; i++) {
//                mve::Vector3i adj_pos = pos + direction_vector(static_cast<Direction>(i));
//                std::optional<uint8_t> adj_block = block_at(adj_pos);
//                if (adj_block.has_value() && is_transparent(adj_block.value())
//                    && light_val > lighting_at(adj_pos).value()) {
//                    set_lighting(adj_pos, light_val);
//                    if (light_val != 1 && std::find(prev_list.begin(), prev_list.end(), adj_pos) == prev_list.end()
//                        && std::find(current_list.begin(), current_list.end(), adj_pos) == current_list.end()
//                        && std::find(next_list.begin(), next_list.end(), adj_pos) == next_list.end()) {
//                        next_list.push_back(adj_pos);
//                    }
//                }
//            }
//            prev_list.push_back(pos);
//        }
//        std::swap(current_list, next_list);
//        next_list.clear();
//        light_val--;
//    }
//}

// void WorldData::propagate_light(mve::Vector3i chunk_pos)
//{
//     for_3d(chunk_pos - mve::Vector3i(1), chunk_pos + mve::Vector3i(2), [&](const mve::Vector3i adj_chunk_pos) {
//         if (adj_chunk_pos.z >= -10 && adj_chunk_pos.z < 10) {
//             m_chunk_columns[{ adj_chunk_pos.x, adj_chunk_pos.y }]
//                 .chunk_data_at(adj_chunk_pos)
//                 .for_emissive_block([&](const mve::Vector3i& local_pos) {
//                     spread_light(block_local_to_world(adj_chunk_pos, local_pos));
//                 });
//         }
//     });
// }

// void WorldData::process_chunk_lighting_updates()
//{
//     //    for (mve::Vector3i chunk_pos : m_chunk_lighting_update_list) {
//     //        m_chunks[chunk_pos].reset_lighting();
//     //    }
//     for (mve::Vector3i chunk_pos : m_chunk_lighting_update_list) {
//         propagate_light(chunk_pos);
//     }
//     m_chunk_lighting_update_list.clear();
// }
