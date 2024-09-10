#pragma once

#include <mve/window.hpp>

#include "../common/fixed_loop.hpp"
// #include "../server/server.hpp"
#include "ui_pipeline.hpp"
#include "world.hpp"
#include "world_renderer.hpp"

namespace app {

class App {
public:
    App();

    ~App();

    App(const App& other) = delete;
    App& operator=(const App& other) = delete;

    void main_loop();

private:
    // void handle_networking() const;

    void draw();

    mve::Window m_window;
    mve::Renderer m_renderer;
    // Server m_server;
    // ENetHost* m_client;
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