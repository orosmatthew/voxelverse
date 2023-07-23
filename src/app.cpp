#include "app.hpp"

#include <fstream>

#include "logger.hpp"

namespace app {

App::App()
    : m_window("Voxelverse", mve::Vector2i(800, 600))
    , m_renderer(m_window, "Voxelverse", 0, 1, 0)
    , m_ui_pipeline(m_renderer)
    , m_text_pipeline(m_renderer, 36)
    , m_world(m_renderer, m_ui_pipeline, m_text_pipeline, 32)
    , m_world_framebuffer(m_renderer.create_framebuffer([this]() {
        m_ui_pipeline.update_framebuffer_texture(
            m_world_framebuffer.texture(), m_renderer.framebuffer_size(m_world_framebuffer));
    }))
    , m_fixed_loop(60.0f)
    , m_begin_time(std::chrono::high_resolution_clock::now())
    , m_current_frame_count(0)
{
    LOG->set_level(spdlog::level::info);
    m_window.set_min_size({ 800, 600 });
    m_window.disable_cursor();

    auto resize_func = [&](mve::Vector2i new_size) {
        m_renderer.resize(m_window);
        m_world.resize(m_renderer.extent());
        m_ui_pipeline.resize();
        m_text_pipeline.resize();
        draw();
    };

    m_window.set_resize_callback(resize_func);

    std::invoke(resize_func, m_window.size());
}

void App::draw()
{
    m_renderer.begin_frame(m_window);

    m_renderer.begin_render_pass_framebuffer(m_world_framebuffer);

    m_world.draw();

    m_renderer.end_render_pass_framebuffer(m_world_framebuffer);

    m_renderer.begin_render_pass_present();

    m_ui_pipeline.draw_world();

    m_renderer.end_render_pass_present();

    m_renderer.end_frame(m_window);
}

void App::main_loop()
{
    while (!m_window.should_close() && !m_world.should_exit()) {
        m_window.poll_events();

        m_fixed_loop.update(20, [&]() { m_world.fixed_update(m_window); });

        m_world.update(m_window, m_fixed_loop.blend());

        if (m_window.is_key_pressed(mve::Key::enter) && m_window.is_key_down(mve::Key::left_alt)) {
            if (!m_window.is_fullscreen()) {
                m_window.fullscreen(true);
            }
            else {
                m_window.windowed();
            }
        }

        m_world.update_debug_fps(m_frame_count);

        draw();

        std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::microseconds>(end_time - m_begin_time).count() >= 1000000) {
            m_begin_time = std::chrono::high_resolution_clock::now();
            m_frame_count = m_current_frame_count;
            m_current_frame_count = 0;
        }
        m_current_frame_count++;
    }
}

}
