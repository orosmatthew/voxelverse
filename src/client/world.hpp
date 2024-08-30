#pragma once

#include <mve/renderer.hpp>

#include "chunk_controller.hpp"
#include "text_pipeline.hpp"
#include "ui/hud.hpp"
#include "ui/pause_menu.hpp"
#include "world_data.hpp"
#include "world_generator.hpp"
#include "world_renderer.hpp"

// TODO: Make hotbar blocks 3D

class ChunkData;
class UIPipeline;

class World {
public:
    World(mve::Renderer& renderer, UIPipeline& ui_pipeline, TextPipeline& text_pipeline, int render_distance);

    void set_render_distance(const int distance)
    {
        m_render_distance = distance;
        m_chunk_controller.set_render_distance(distance);
    }

    void fixed_update(const mve::Window& window);

    void update(mve::Window& window, float blend, mve::Renderer& renderer);

    void resize(nnm::Vector2i extent);

    void draw();

    void update_debug_fps(const int fps)
    {
        m_hud.update_debug_fps(fps);
    }

    [[nodiscard]] nnm::Vector3i player_block_pos() const;

    [[nodiscard]] nnm::Vector3i player_chunk_pos() const;

    [[nodiscard]] std::optional<const ChunkData*> chunk_data_at(nnm::Vector3i chunk_pos) const;

    [[nodiscard]] const WorldData& world_data() const
    {
        return m_world_data;
    }

    [[nodiscard]] bool should_exit() const
    {
        return m_should_exit;
    }

private:
    enum class FocusState { world, console, pause };

    void update_world(mve::Window& window);

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