#pragma once

#include <cstdint>

#include "mve/math/math.hpp"

class WorldData;
class WorldGenerator;
class WorldRenderer;

class ChunkController {
public:
    void update(
        WorldData& world_data,
        WorldGenerator& world_generator,
        WorldRenderer& world_renderer,
        mve::Vector3i player_chunk);

    inline ChunkController& set_render_distance(int dist)
    {
        m_render_distance = dist;
        return *this;
    }

    inline ChunkController& set_mesh_updates_per_frame(int updates)
    {
        m_mesh_updates_per_frame = updates;
        return *this;
    }

private:
    enum ChunkFlagBits {
        flag_has_mesh = (1 << 0),
        flag_has_data = (1 << 1),
        flag_queued_delete = (1 << 2),
        flag_queued_mesh = (1 << 3),
    };

    template <typename T, typename U>
    bool contains_flag(T val, U flag)
    {
        return (val & flag) != 0;
    }

    template <typename T, typename U>
    void enable_flag(T& val, U flag)
    {
        val |= flag;
    }

    template <typename T, typename U>
    void disable_flag(T& val, U flag)
    {
        val &= ~flag;
    }

    struct ChunkState {
        uint8_t m_flags {};
        int neighbors = 0;
    };

    void sort_cols();

    mve::Vector2i m_player_chunk_col = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
    std::vector<mve::Vector2i> m_sorted_cols {};
    std::unordered_map<mve::Vector2i, ChunkState> m_chunk_states;
    int m_render_distance;
    int m_mesh_updates_per_frame;
};