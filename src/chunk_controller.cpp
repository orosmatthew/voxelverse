#include "chunk_controller.hpp"
#include "mve/math/math.hpp"
#include "world_data.hpp"
#include "world_generator.hpp"
#include "world_renderer.hpp"

void ChunkController::update(
    WorldData& world_data,
    WorldGenerator& world_generator,
    WorldRenderer& world_renderer,
    const mve::Vector3i player_chunk)
{
    if (const mve::Vector2i player_chunk_col = { player_chunk.x, player_chunk.y };
        player_chunk_col != m_player_chunk_col) {
        m_player_chunk_col = player_chunk_col;
        sort_cols();
    }

    int chunk_count = 0;
    for (mve::Vector2i col_pos : m_sorted_cols) {
        auto& [m_flags, neighbors] = m_chunk_states.at(col_pos);
        uint8_t& flags = m_flags;
        // if (col_pos == player_chunk_col && contains_flag(flags, flag_has_data)) {
        //     auto thing = world_data.chunk_column_data_at(col_pos);
        // }
        if (!contains_flag(flags, flag_has_data)) {
            if (!world_data.try_load_chunk_column_from_save(col_pos)) {
                if (!world_data.contains_column(col_pos)) {
                    world_data.create_chunk_column(col_pos);
                }
                world_generator.generate_chunk(world_data, col_pos);
                if (world_data.chunk_column_data_at(col_pos).gen_level() == ChunkColumn::GenLevel::generated) {
                    world_data.queue_save_chunk(col_pos);
                }
            }
            for_2d({ -1, -1 }, { 2, 2 }, [&](const mve::Vector2i neighbor) {
                if (neighbor == mve::Vector2i(0, 0)) {
                    return;
                }
                // ReSharper disable once CppUseStructuredBinding
                ChunkState& neighbor_state = m_chunk_states[col_pos + neighbor];
                neighbor_state.neighbors++;
                if (contains_flag(neighbor_state.m_flags, flag_has_data) && neighbor_state.neighbors == 8) {
                    enable_flag(neighbor_state.m_flags, flag_queued_mesh);
                }
            });
            enable_flag(flags, flag_has_data);
            if (neighbors == 8) {
                enable_flag(flags, flag_queued_mesh);
            }
            chunk_count++;
        }

        if (!contains_flag(flags, flag_has_mesh) && contains_flag(flags, flag_queued_mesh)) {
            for (int h = -10; h < 10; h++) {
                world_data.push_chunk_lighting_update({ col_pos.x, col_pos.y, h });
                world_renderer.push_mesh_update({ col_pos.x, col_pos.y, h });
            }
            enable_flag(flags, flag_has_mesh);
            disable_flag(flags, flag_queued_mesh);
            chunk_count++;
        }
        if (chunk_count > m_mesh_updates_per_frame) {
            break;
        }
    }

    //    world_data.process_chunk_lighting_updates();
    world_renderer.process_mesh_updates(world_data);

    chunk_count = 0;

    for (int i = static_cast<int>(m_sorted_cols.size()) - 1; i >= 0; i--) {
        const mve::Vector2i pos = m_sorted_cols[i];
        auto& [m_flags, neighbors] = m_chunk_states.at(pos);
        if (!contains_flag(m_flags, flag_queued_delete)) {
            continue;
        }

        m_sorted_cols.erase(std::ranges::find(m_sorted_cols, pos));

        if (contains_flag(m_flags, flag_has_data)) {
            for_2d({ -1, -1 }, { 2, 2 }, [&](const mve::Vector2i neighbor) {
                if (neighbor == mve::Vector2i(0, 0)) {
                    return;
                }
                if (m_chunk_states.contains(pos + neighbor)) {
                    m_chunk_states.at(pos + neighbor).neighbors--;
                }
            });
        }

        if (contains_flag(m_flags, flag_has_data)) {
            world_data.remove_chunk_column({ pos.x, pos.y });
        }
        for (int h = -10; h < 10; h++) {
            if (contains_flag(m_flags, flag_has_mesh)) {
                world_renderer.remove_data({ pos.x, pos.y, h });
            }
        }
        if (contains_flag(m_flags, flag_has_mesh)) {
            chunk_count++;
        }
        disable_flag(m_flags, flag_has_data);
        disable_flag(m_flags, flag_has_mesh);
        disable_flag(m_flags, flag_queued_mesh);
        disable_flag(m_flags, flag_queued_delete);
        if (m_chunk_states.at(pos).neighbors == 0) {
            m_chunk_states.erase(pos);
        }
        if (chunk_count > m_mesh_updates_per_frame) {
            break;
        }
    }
}
void ChunkController::sort_cols()
{
    m_sorted_cols.clear();
    m_sorted_cols.reserve(m_chunk_states.size());
    for (auto& [pos, data] : m_chunk_states) {
        if (mve::abs(mve::sqrd(pos.x - m_player_chunk_col.x) + mve::sqrd(pos.y - m_player_chunk_col.y))
            > mve::sqrd(m_render_distance)) {
            enable_flag(data.m_flags, flag_queued_delete);
        }
    }
    for_2d(
        mve::Vector2i(-m_render_distance, -m_render_distance)
            + mve::Vector2i(m_player_chunk_col.x, m_player_chunk_col.y),
        mve::Vector2i(m_render_distance, m_render_distance) + mve::Vector2i(m_player_chunk_col.x, m_player_chunk_col.y),
        [&](const mve::Vector2i pos) {
            if (mve::abs(mve::sqrd(pos.x - m_player_chunk_col.x) + mve::sqrd(pos.y - m_player_chunk_col.y))
                <= mve::sqrd(m_render_distance)) {
                if (!m_chunk_states.contains(pos)) {
                    m_chunk_states[pos] = {};
                }
                else {
                    disable_flag(m_chunk_states[pos].m_flags, flag_queued_delete);
                }
            }
        });
    for (auto& [pos, data] : m_chunk_states) {
        m_sorted_cols.push_back(pos);
    }
    std::ranges::sort(m_sorted_cols, [&](const mve::Vector2i& a, const mve::Vector2i& b) {
        return distance_sqrd(
                   mve::Vector2(a),
                   mve::Vector2(static_cast<float>(m_player_chunk_col.x), static_cast<float>(m_player_chunk_col.y)))
            < distance_sqrd(
                   mve::Vector2(b),
                   mve::Vector2(static_cast<float>(m_player_chunk_col.x), static_cast<float>(m_player_chunk_col.y)));
    });
}
