#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "common.hpp"

#include <nnm/nnm.hpp>

class WorldData;
class WorldGenerator;
class WorldRenderer;

class ChunkController {
public:
    void update(
        WorldData& world_data,
        const WorldGenerator& world_generator,
        WorldRenderer& world_renderer,
        nnm::Vector3i player_chunk);

    ChunkController& set_render_distance(const int dist)
    {
        m_render_distance = dist;
        return *this;
    }

    ChunkController& set_mesh_updates_per_frame(const int updates)
    {
        m_mesh_updates_per_frame = updates;
        return *this;
    }

    void queue_recreate_mesh(nnm::Vector2i chunk_pos);

private:
    enum ChunkFlagBits {
        flag_has_mesh = 1 << 0,
        flag_is_generated = 1 << 1,
        flag_queued_mesh = 1 << 2,
    };

    template <typename T, typename U>
    static bool contains_flag(T val, U flag)
    {
        return (val & flag) != 0;
    }

    template <typename T, typename U>
    static void enable_flag(T& val, U flag)
    {
        val |= flag;
    }

    template <typename T, typename U>
    static void disable_flag(T& val, U flag)
    {
        val &= ~flag;
    }

    struct ChunkState {
        uint8_t flags {};
        int generated_neighbors = 0;
    };

    void on_player_chunk_change();

    inline static const std::array<nnm::Vector2i, 4> sc_nbor_offsets { { { 0, 1 }, { 0, -1 }, { 1, 0 }, { -1, 0 } } };
    static constexpr auto sc_full_nbors = static_cast<int>(sc_nbor_offsets.size());

    nnm::Vector2i m_player_chunk_col = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
    std::vector<nnm::Vector2i> m_sorted_chunks_in_range {};
    std::unordered_map<nnm::Vector2i, ChunkState> m_chunk_states;
    int m_render_distance = 0;
    int m_mesh_updates_per_frame = 0;
};