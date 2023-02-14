#pragma once

#include "renderer.hpp"
#include "world_renderer.hpp"

// TODO: Chunk unloading
// TODO: Add hotbar scrolling
// TODO: Make hotbar functional

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
    mve::Window* m_window;
    mve::Renderer* m_renderer;
    UIRenderer* m_ui_renderer;
    WorldRenderer m_world_renderer;
    WorldGenerator m_world_generator;
    WorldData m_world_data;
    int m_mesh_updates_per_frame;
    Camera m_camera;
    std::vector<mve::Vector3i> m_chunk_data_queue;
    std::vector<mve::Vector3i> m_chunk_mesh_queue;
    std::vector<mve::Vector3i> m_chunk_removal_queue;
    mve::Vector3i m_camera_chunk = { -1, -1, -1 };
    int m_render_distance;
    int m_current_hotbar_select;
};