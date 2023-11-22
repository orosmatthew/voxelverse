#include "chunk_controller.hpp"

#include "lighting.hpp"
#include "mve/math/math.hpp"
#include "world_data.hpp"
#include "world_generator.hpp"
#include "world_renderer.hpp"

void ChunkController::update(
    WorldData& world_data,
    const WorldGenerator& world_generator,
    WorldRenderer& world_renderer,
    const mve::Vector3i player_chunk)
{
    if (const mve::Vector2i player_chunk_col = { player_chunk.x, player_chunk.y };
        player_chunk_col != m_player_chunk_col) {
        world_data.set_player_chunk(player_chunk_col);
        m_player_chunk_col = player_chunk_col;
        on_player_chunk_change();
    }

    int chunk_count = 0;
    for (mve::Vector2i col_pos : m_sorted_cols) {
        auto& [flags, neighbors] = m_chunk_states.at(col_pos);
        if (!contains_flag(flags, flag_is_generated)) {
            if (!world_data.try_load_chunk_column_from_save(col_pos)) {
                if (!world_data.contains_column(col_pos)) {
                    world_data.create_chunk_column(col_pos);
                }
                world_generator.generate_chunk(world_data, col_pos);
                if (world_data.chunk_column_data_at(col_pos).gen_level() == ChunkColumn::GenLevel::generated) {
                    world_data.queue_save_chunk(col_pos);
                }
            }
            for (mve::Vector2i offset : sc_nbor_offsets) {
                // ReSharper disable once CppUseStructuredBinding
                ChunkState& neighbor_state = m_chunk_states[col_pos + offset];
                neighbor_state.generated_neighbors++;
                if (contains_flag(neighbor_state.flags, flag_is_generated)
                    && neighbor_state.generated_neighbors == sc_full_nbors) {
                    enable_flag(neighbor_state.flags, flag_queued_mesh);
                }
            }
            enable_flag(flags, flag_is_generated);
            if (neighbors == sc_full_nbors) {
                enable_flag(flags, flag_queued_mesh);
            }
            chunk_count++;
        }

        if (!contains_flag(flags, flag_has_mesh) && contains_flag(flags, flag_queued_mesh)) {
            apply_sunlight(world_data.chunk_column_data_at({ col_pos.x, col_pos.y }));
            for (int h = -10; h < 10; h++) {
                // world_data.propagate_light({ col_pos.x, col_pos.y, h });
                // world_data.push_chunk_lighting_update({ col_pos.x, col_pos.y, h });
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

    world_renderer.process_mesh_updates(world_data);

    chunk_count = 0;
    while (std::optional<mve::Vector2i> culled_chunk
           = world_data.try_cull_chunk(static_cast<float>(m_render_distance))) {
        if (!m_chunk_states.contains(culled_chunk.value())) {
            continue;
        }
        uint8_t& flags = m_chunk_states.at(culled_chunk.value()).flags;
        if (contains_flag(flags, flag_is_generated)) {
            for (mve::Vector2i offset : sc_nbor_offsets) {
                if (const mve::Vector2i neighbor = culled_chunk.value() + offset; m_chunk_states.contains(neighbor)) {
                    if (--m_chunk_states.at(neighbor).generated_neighbors == 0) {
                        m_chunk_states.erase(neighbor);
                    }
                }
            }
            disable_flag(flags, flag_is_generated);
        }
        if (contains_flag(flags, flag_has_mesh)) {
            for (int h = -10; h < 10; h++) {
                world_renderer.remove_data({ culled_chunk.value().x, culled_chunk.value().y, h });
            }
            disable_flag(flags, flag_has_mesh);
        }

        disable_flag(flags, flag_queued_mesh);
        if (++chunk_count > m_mesh_updates_per_frame) {
            break;
        }
    }
}
void ChunkController::on_player_chunk_change()
{
    m_sorted_cols.clear();
    m_sorted_cols.reserve(m_chunk_states.size());
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
