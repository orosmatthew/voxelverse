#pragma once

#include "mve/math/math.hpp"
#include "mve/window.hpp"
#include "player.hpp"
#include "ui_pipeline.hpp"
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
    UIPipeline m_ui_pipeline;
    TextPipeline m_text_pipeline;
    World m_world;
    mve::Framebuffer m_world_framebuffer;
    util::FixedLoop m_fixed_loop;
    std::chrono::high_resolution_clock::time_point m_begin_time;
    int m_frame_count = 0;
    int m_current_frame_count = 0;
};

}