#include "app.hpp"

#include <fstream>
// #include <leveldb/db.h>

#include <cereal/archives/portable_binary.hpp>
#include <lz4hc.h>

#include "logger.hpp"
#include "mve/common.hpp"

namespace app {

App::App()
    : m_window("Mini Vulkan Engine", mve::Vector2i(800, 600))
    , m_renderer(m_window, "Vulkan Testing", 0, 0, 1)
    , m_ui_renderer(m_renderer)
    , m_world(m_window, m_renderer, m_ui_renderer, 32)
    , m_world_framebuffer(m_renderer.create_framebuffer([this]() {
        m_ui_renderer.update_framebuffer_texture(
            m_world_framebuffer.texture(), m_renderer.framebuffer_size(m_world_framebuffer));
    }))
    , m_fixed_loop(60.0f)
    , m_cursor_captured(true)
    , m_begin_time(std::chrono::high_resolution_clock::now())
    , m_current_frame_count(0)
{
    LOG->set_level(spdlog::level::info);
    m_window.set_min_size({ 800, 600 });
    m_window.disable_cursor();

    auto resize_func = [&](mve::Vector2i new_size) {
        m_renderer.resize(m_window);
        m_world.resize();
        m_ui_renderer.resize();
        draw();
    };

    m_window.set_resize_callback(resize_func);

    std::invoke(resize_func, m_window.size());

    m_ui_renderer.update_gpu_name(m_renderer.gpu_name());
}

void App::draw()
{
    m_renderer.begin_frame(m_window);

    m_renderer.begin_render_pass_framebuffer(m_world_framebuffer);

    m_world.draw();

    m_renderer.end_render_pass_framebuffer(m_world_framebuffer);

    m_renderer.begin_render_pass_present();

    m_ui_renderer.draw();

    m_renderer.end_render_pass_present();

    m_renderer.end_frame(m_window);
}

void App::save_world()
{
    const WorldData& world_data = m_world.world_data();
    std::stringstream serial_stream;
    {
        cereal::PortableBinaryOutputArchive archive_out(serial_stream);
        archive_out(world_data);
    }
    std::string serial_data = serial_stream.str();
    std::vector<char> compressed_data(LZ4_compressBound(serial_data.size()));
    int compressed_size
        = LZ4_compress_HC(serial_data.data(), compressed_data.data(), serial_data.size(), compressed_data.size(), 8);
    //    int compressed_size
    //        = LZ4_compress_default(serial_data.data(), compressed_data.data(), serial_data.size(),
    //        compressed_data.size());
    MVE_ASSERT(compressed_size > 0, "[App] LZ4 compression error")
    compressed_data.resize(compressed_size);
    std::ofstream save_file("save.bin", std::ios::out | std::ios::binary);
    save_file.write(compressed_data.data(), compressed_data.size());
    //            save_file.write(serial_data.c_str(), serial_data.size());
    save_file.close();
}

void App::main_loop()
{
    while (!m_window.should_close()) {
        m_window.poll_events();

        m_fixed_loop.update(20, [&]() { m_world.fixed_update(); });

        m_world.update(m_cursor_captured, m_fixed_loop.blend());

        if (m_window.is_key_pressed(mve::Key::escape)) {
            break;
        }

        if (m_window.is_key_pressed(mve::Key::f)) {
            if (!m_window.is_fullscreen()) {
                m_window.fullscreen(true);
            }
            else {
                m_window.windowed();
            }
        }

        if (m_window.is_key_pressed(mve::Key::c)) {
            if (m_cursor_captured) {
                m_window.enable_cursor();
                m_cursor_captured = false;
            }
            else {
                m_window.disable_cursor();
                m_cursor_captured = true;
            }
        }

        if (m_window.is_key_pressed(mve::Key::f3)) {
            m_ui_renderer.is_debug_enabled() ? m_ui_renderer.disable_debug() : m_ui_renderer.enable_debug();
        }

        if (m_window.is_key_pressed(mve::Key::o)) {
            save_world();
        }

        m_ui_renderer.update_fps(m_frame_count);
        m_ui_renderer.update_player_block_pos(m_world.player_block_pos());
        m_ui_renderer.update_player_chunk_pos(m_world.player_chunk_pos());

        draw();

        std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::microseconds>(end_time - m_begin_time).count() >= 1000000) {
            m_begin_time = std::chrono::high_resolution_clock::now();
            LOG->info("Framerate: {}", m_current_frame_count);
            m_frame_count = m_current_frame_count;
            m_current_frame_count = 0;
        }

        m_current_frame_count++;
    }
}
}
