#pragma once

#include "mve/renderer.hpp"
#include "world_renderer.hpp"
#include "world_generator.hpp"

// TODO: Chunk unloading
// TODO: Make hotbar blocks 3D

class UIRenderer;

class World {
public:
    World(mve::Window& window, mve::Renderer& renderer, UIRenderer& ui_renderer, int render_distance);

    inline void set_render_distance(int distance)
    {
        m_render_distance = distance;
    }

    void fixed_update();

    void update(bool mouse_captured, float blend);

    void resize();

    void draw();

private:
    struct ChunkState {
        bool has_mesh = false;
        bool can_mesh = false;
        bool has_data = false;
        bool should_delete = false;
        int neighbors = 0;
    };
    mve::Window* m_window;
    mve::Renderer* m_renderer;
    UIRenderer* m_ui_renderer;
    WorldRenderer m_world_renderer;
    WorldGenerator m_world_generator;
    WorldData m_world_data;
    int m_mesh_updates_per_frame;
    Camera m_camera;
    std::unordered_map<mve::Vector2i, ChunkState> m_chunk_states;
    std::vector<mve::Vector2i> m_sorted_chunks;
    mve::Vector2i m_camera_chunk = { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
    int m_render_distance;
    int m_current_hotbar_select = 0;
    std::unordered_map<int, uint8_t> m_hotbar_blocks {};
};