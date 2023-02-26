#pragma once

#include "camera.hpp"
#include "mve/math/math.hpp"
#include "mve/window.hpp"
#include "ui_renderer.hpp"
#include "util/fixed_loop.hpp"
#include "world.hpp"
#include "world_data.hpp"
#include "world_renderer.hpp"

namespace app {

class App {
public:
    App();

    void main_loop();

private:

    void draw();

    mve::Window m_window;
    mve::Renderer m_renderer;
    UIRenderer m_ui_renderer;
    World m_world;
    mve::Framebuffer m_world_framebuffer;
    util::FixedLoop m_fixed_loop;
    bool m_cursor_captured;
    std::chrono::high_resolution_clock::time_point m_begin_time;
    int m_frame_count;
};

}