#pragma once

#include "chunk_controller.hpp"
#include "mve/renderer.hpp"
#include "text_pipeline.hpp"
#include "ui/crosshair.hpp"
#include "ui/debug_overlay.hpp"
#include "ui/hotbar.hpp"
#include "ui/hud.hpp"
#include "ui/pause_menu.hpp"
#include "world_generator.hpp"
#include "world_renderer.hpp"

// TODO: Make hotbar blocks 3D

class UIPipeline;

class World {
public:
    World(mve::Renderer& renderer, UIPipeline& ui_pipeline, TextPipeline& text_pipeline, int render_distance);

    inline void set_render_distance(int distance)
    {
        m_render_distance = distance;
        m_chunk_controller.set_render_distance(distance);
    }

    void fixed_update(const mve::Window& window);

    void update(mve::Window& window, float blend);

    void resize(mve::Vector2i extent);

    void draw();

    inline void update_debug_fps(int fps)
    {
        m_hud.update_debug_fps(fps);
    }

    [[nodiscard]] mve::Vector3i player_block_pos() const;

    [[nodiscard]] mve::Vector3i player_chunk_pos() const;

    [[nodiscard]] std::optional<const ChunkData*> chunk_data_at(mve::Vector3i chunk_pos) const;

    [[nodiscard]] inline const WorldData& world_data() const
    {
        return m_world_data;
    }

    [[nodiscard]] inline bool should_exit() const
    {
        return m_should_exit;
    }

private:
    enum class FocusState { world, console, pause };

    void update_world(mve::Window& window, float blend);

    WorldRenderer m_world_renderer;
    WorldGenerator m_world_generator;
    WorldData m_world_data;
    Player m_player;
    ChunkController m_chunk_controller {};
    int m_render_distance;
    HUD m_hud;
    PauseMenu m_pause_menu;
    std::chrono::time_point<std::chrono::steady_clock> m_last_place_time;
    std::chrono::time_point<std::chrono::steady_clock> m_last_break_time;
    FocusState m_focus;
    bool m_should_exit;
};