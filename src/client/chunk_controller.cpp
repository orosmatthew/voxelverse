#include "chunk_controller.hpp"

#include <ranges>

#include "world_data.hpp"
#include "world_generator.hpp"
#include "world_renderer.hpp"

void ChunkController::update(
    WorldData& world_data,
    const WorldGenerator& world_generator,
    WorldRenderer& world_renderer,
    const nnm::Vector3i player_chunk)
{
    if (const nnm::Vector2i player_chunk_col = { player_chunk.x, player_chunk.y };
        player_chunk_col != m_player_chunk_col) {
        world_data.set_player_chunk(player_chunk_col);
        m_player_chunk_col = player_chunk_col;
        on_player_chunk_change();
    }

    int chunk_count = 0;
    for (const nnm::Vector2i col_pos : m_sorted_chunks_in_range) {
        auto& [flags, neighbors] = m_chunk_states.at(col_pos);
        if (!contains_flag(flags, flag_is_generated)) {
            if (!world_data.contains_column(col_pos)) {
                world_data.create_or_load_chunk(col_pos);
            }
            if (world_data.chunk_column_data_at(col_pos).gen_level() < ChunkColumn::generated) {
                world_generator.generate_chunk(world_data, col_pos);
                world_data.queue_save_chunk(col_pos);
            }
            for (const nnm::Vector2i offset : sc_nbor_offsets) {
                if (!m_chunk_states.contains(col_pos + offset)) {
                    m_chunk_states.insert({ col_pos + offset, {} });
                }
                // ReSharper disable once CppUseStructuredBinding
                ChunkState& neighbor_state = m_chunk_states.at(col_pos + offset);
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

        if (contains_flag(flags, flag_queued_mesh)) {
            for (int h = -10; h < 10; h++) {
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
    while (std::optional<nnm::Vector2i> culled_chunk
           = world_data.try_cull_chunk(static_cast<float>(m_render_distance) + 3.0f)) {
        if (!m_chunk_states.contains(culled_chunk.value())) {
            continue;
        }
        uint8_t& flags = m_chunk_states.at(culled_chunk.value()).flags;
        if (contains_flag(flags, flag_is_generated)) {
            for (nnm::Vector2i offset : sc_nbor_offsets) {
                if (const nnm::Vector2i neighbor = culled_chunk.value() + offset; m_chunk_states.contains(neighbor)) {
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

void ChunkController::queue_recreate_mesh(const nnm::Vector2i chunk_pos)
{
    if (m_chunk_states.contains(chunk_pos)) {
        if (uint8_t& flags = m_chunk_states.at(chunk_pos).flags;
            contains_flag(flags, flag_is_generated) && contains_flag(flags, flag_has_mesh)) {
            enable_flag(flags, flag_queued_mesh);
        }
    }
}

void ChunkController::on_player_chunk_change()
{
    m_sorted_chunks_in_range.clear();
    for_2d(
        nnm::Vector2i(-m_render_distance, -m_render_distance)
            + nnm::Vector2i(m_player_chunk_col.x, m_player_chunk_col.y),
        nnm::Vector2i(m_render_distance, m_render_distance) + nnm::Vector2i(m_player_chunk_col.x, m_player_chunk_col.y),
        [&](const nnm::Vector2i pos) {
            if (nnm::abs(nnm::sqrd(pos.x - m_player_chunk_col.x) + nnm::sqrd(pos.y - m_player_chunk_col.y))
                <= nnm::sqrd(m_render_distance)) {
                if (!m_chunk_states.contains(pos)) {
                    m_chunk_states[pos] = {};
                }
            }
        });
    for (const auto& pos : m_chunk_states | std::views::keys) {
        m_sorted_chunks_in_range.push_back(pos);
    }
    std::ranges::sort(m_sorted_chunks_in_range, [&](const nnm::Vector2i& a, const nnm::Vector2i& b) {
        return nnm::Vector2f(a).distance_sqrd(
                   nnm::Vector2f(static_cast<float>(m_player_chunk_col.x), static_cast<float>(m_player_chunk_col.y)))
            < nnm::Vector2f(b).distance_sqrd(
                nnm::Vector2(static_cast<float>(m_player_chunk_col.x), static_cast<float>(m_player_chunk_col.y)));
    });
}
