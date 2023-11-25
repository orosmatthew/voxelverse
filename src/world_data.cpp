#include "world_data.hpp"

#include "lighting.hpp"

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

void WorldData::propagate_light(const mve::Vector3i chunk_pos)
{
    static std::vector<std::pair<mve::Vector3i, uint8_t>> queue;
    queue.clear();

    const std::array<mve::Vector3i, 6> adjacent
        = { { { 0, 0, 1 }, { 0, 0, -1 }, { 0, 1, 0 }, { 0, -1, 0 }, { 1, 0, 0 }, { -1, 0, 0 } } };

    std::array<mve::Vector3i, 26> surr_pos {};
    {
        int i = 0;
        for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](const mve::Vector3i pos) {
            surr_pos[i] = pos;
            ++i;
        });
    }

    ChunkData& current_chunk_data = chunk_data_at(chunk_pos);
    std::array<std::optional<ChunkData*>, 26> surr_chunks {};
    for (int i = 0; i < surr_pos.size(); ++i) {
        if (contains_chunk(chunk_pos + surr_pos[i])) {
            surr_chunks[i] = &chunk_data_at(chunk_pos + surr_pos[i]);
        }
    }

    auto fast_lighting_at = [&](const mve::Vector3i pos) -> std::optional<uint8_t> {
        const mve::Vector3i offset = chunk_pos_from_block_pos(pos) - chunk_pos;
        const mve::Vector3i local_pos = block_world_to_local(pos);
        if (offset == mve::Vector3i(0, 0, 0)) {
            return current_chunk_data.lighting_at(local_pos);
        }
        for (int i = 0; i < surr_chunks.size(); ++i) {
            if (offset == surr_pos[i]) {
                return surr_chunks[i].has_value()
                    ? surr_chunks[i].value()->lighting_at(local_pos)
                    : std::optional<uint8_t> {};
            }
        }
        MVE_VAL_ASSERT(false, "Unreachable");
        return {};
    };

    auto fast_block_at = [&](const mve::Vector3i pos) -> std::optional<uint8_t> {
        const mve::Vector3i offset = chunk_pos_from_block_pos(pos) - chunk_pos;
        const mve::Vector3i local_pos = block_world_to_local(pos);
        if (offset == mve::Vector3i(0, 0, 0)) {
            return current_chunk_data.get_block(local_pos);
        }
        for (int i = 0; i < surr_chunks.size(); ++i) {
            if (offset == surr_pos[i]) {
                return surr_chunks[i].has_value()
                    ? surr_chunks[i].value()->get_block(local_pos)
                    : std::optional<uint8_t> {};
            }
        }
        MVE_VAL_ASSERT(false, "Unreachable");
        return {};
    };

    auto fast_set_lighting = [&](const mve::Vector3i pos, const uint8_t val) {
        const mve::Vector3i offset = chunk_pos_from_block_pos(pos) - chunk_pos;
        const mve::Vector3i local_pos = block_world_to_local(pos);
        if (offset == mve::Vector3i(0, 0, 0)) {
            return current_chunk_data.set_lighting(local_pos, val);
        }
        for (int i = 0; i < surr_chunks.size(); ++i) {
            if (offset == surr_pos[i]) {
                surr_chunks[i].value()->set_lighting(local_pos, val);
                return;
            }
        }
        MVE_VAL_ASSERT(false, "Unreachable");
    };

    for_3d({ 0, 0, 0 }, { 16, 16, 16 }, [&](const mve::Vector3i pos) {
        const mve::Vector3i world_pos = block_local_to_world(chunk_pos, pos);
        if (const std::optional<uint8_t> block = fast_block_at(world_pos);
            fast_lighting_at(world_pos) >= 15 || (block.has_value() && block.value() == 9)) {
            queue.emplace_back(world_pos, 15);
        }
    });

    while (!queue.empty()) {
        const auto [pos, prev_val] = queue.back();
        queue.pop_back();
        for (const mve::Vector3i offset : adjacent) {
            const mve::Vector3i adj_pos = pos + offset;
            const std::optional<uint8_t> current_lighting = fast_lighting_at(adj_pos);
            const std::optional<uint8_t> block_type = fast_block_at(adj_pos);
            if (block_type.has_value() && current_lighting.has_value() && current_lighting < prev_val - 1
                && is_transparent(block_type.value())) {
                fast_set_lighting(adj_pos, prev_val - 1);
                if (prev_val - 1 > 1) {
                    queue.emplace_back(adj_pos, prev_val - 1);
                }
            }
        }
    }
}

void WorldData::refresh_lighting(const mve::Vector3i chunk_pos)
{
    for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](const mve::Vector3i offset) {
        if (contains_chunk(chunk_pos + offset)) {
            chunk_data_at(chunk_pos + offset).reset_lighting(0);
        }
    });

    for_2d({ -1, -1 }, { 2, 2 }, [&](const mve::Vector2i offset) {
        if (m_chunk_columns.contains({ chunk_pos.x + offset.x, chunk_pos.y + offset.y })) {
            apply_sunlight(chunk_column_data_at({ chunk_pos.x + offset.x, chunk_pos.y + offset.y }));
        }
    });

    for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](const mve::Vector3i offset) {
        if (contains_chunk(chunk_pos + offset)) {
            propagate_light(chunk_pos + offset);
        }
    });
}

// void WorldData::propagate_light(const mve::Vector3i chunk_pos)
// {
//     for_3d(chunk_pos - mve::Vector3i(1), chunk_pos + mve::Vector3i(2), [&](const mve::Vector3i adj_chunk_pos) {
//         if (adj_chunk_pos.z >= -10 && adj_chunk_pos.z < 10) {
//             if (m_chunk_columns.contains({ adj_chunk_pos.x, adj_chunk_pos.y })) {
//                 m_chunk_columns[{ adj_chunk_pos.x, adj_chunk_pos.y }]
//                     .chunk_data_at(adj_chunk_pos)
//                     .for_emissive_block([&](const mve::Vector3i& local_pos) {
//                         spread_light(block_local_to_world(adj_chunk_pos, local_pos));
//                     });
//             }
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
