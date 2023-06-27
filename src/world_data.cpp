#include "world_data.hpp"

#include <cereal/archives/portable_binary.hpp>

WorldData::WorldData()
    : m_save(16 * 1024 * 1024, "world_data")
{
}

void WorldData::queue_save_chunk(mve::Vector2i pos)
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
    for (mve::Vector2i pos : m_save_queue) {
        m_save.insert<mve::Vector2i, ChunkColumn>(pos, m_chunk_columns.at(pos));
    }
    m_save.submit_batch();
    m_save_queue.clear();
}
bool WorldData::try_load_chunk_column_from_save(mve::Vector2i chunk_pos)
{
    std::optional<ChunkColumn> data = m_save.at<mve::Vector2i, ChunkColumn>(chunk_pos);
    if (!data.has_value()) {
        return false;
    }
    data->set_modified_callback(m_modified_callback);
    m_chunk_columns.insert({ chunk_pos, std::move(*data) });
    return true;
}

// NOTE: Not thread safe
void WorldData::spread_light(const mve::Vector3i light_pos)
{
    static std::vector<mve::Vector3i> prev_list;
    static std::vector<mve::Vector3i> current_list;
    static std::vector<mve::Vector3i> next_list;
    prev_list.clear();
    current_list.clear();
    next_list.clear();

    current_list.push_back(light_pos);

    int light_val = 15;

    while (!current_list.empty()) {
        for (mve::Vector3i pos : current_list) {
            for (int i = 0; i < 6; i++) {
                mve::Vector3i adj_pos = pos + direction_vector(static_cast<Direction>(i));
                std::optional<uint8_t> adj_block = block_at(adj_pos);
                if (adj_block.has_value() && is_transparent(adj_block.value())
                    && light_val > lighting_at(adj_pos).value()) {
                    set_lighting(adj_pos, light_val);
                    if (light_val != 1 && std::find(prev_list.begin(), prev_list.end(), adj_pos) == prev_list.end()
                        && std::find(current_list.begin(), current_list.end(), adj_pos) == current_list.end()
                        && std::find(next_list.begin(), next_list.end(), adj_pos) == next_list.end()) {
                        next_list.push_back(adj_pos);
                    }
                }
            }
            prev_list.push_back(pos);
        }
        std::swap(current_list, next_list);
        next_list.clear();
        light_val--;
    }
}

void WorldData::propagate_light(mve::Vector3i chunk_pos)
{
    for_3d(chunk_pos - mve::Vector3i(1), chunk_pos + mve::Vector3i(2), [&](const mve::Vector3i adj_chunk_pos) {
        if (adj_chunk_pos.z >= -10 && adj_chunk_pos.z < 10) {
            m_chunk_columns[{ adj_chunk_pos.x, adj_chunk_pos.y }]
                .chunk_data_at(adj_chunk_pos)
                .for_emissive_block([&](const mve::Vector3i& local_pos) {
                    spread_light(block_local_to_world(adj_chunk_pos, local_pos));
                });
        }
    });
}

void WorldData::process_chunk_lighting_updates()
{
    //    for (mve::Vector3i chunk_pos : m_chunk_lighting_update_list) {
    //        m_chunks[chunk_pos].reset_lighting();
    //    }
    for (mve::Vector3i chunk_pos : m_chunk_lighting_update_list) {
        propagate_light(chunk_pos);
    }
    m_chunk_lighting_update_list.clear();
}
