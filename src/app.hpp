#pragma once

#include "camera.hpp"
#include "math/math.hpp"
#include "ui_renderer.hpp"
#include "util/fixed_loop.hpp"
#include "window.hpp"
#include "world_data.hpp"
#include "world_renderer.hpp"

namespace app {

class App {
public:
    App();

    void main_loop();

private:
    mve::Window m_window;
    mve::Renderer m_renderer;
    UIRenderer m_ui_renderer;
    mve::Framebuffer m_world_framebuffer;
    util::FixedLoop m_fixed_loop;
    bool m_cursor_captured;
    WorldRenderer m_world_renderer;
    int m_mesh_updates_per_frame;
    bool m_printed;
    std::chrono::high_resolution_clock::time_point m_begin_time;
    int m_frame_count;
    Camera m_camera;
    WorldData m_world_data;
    std::vector<mve::Vector3i> m_chunk_mesh_queue;

};

}