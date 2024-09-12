#include "app.hpp"

#include <fstream>

#include "../common/logger.hpp"
#include "options.hpp"

namespace app {
App::App()
    : m_window("Voxelverse", nnm::Vector2i(800, 600))
    , m_renderer { m_window, "Voxelverse", mve::Version { 0, 1, 0 } }
    , m_ui_pipeline(m_renderer)
    , m_text_pipeline(m_renderer, 36)
    , m_world(m_renderer, m_ui_pipeline, m_text_pipeline, 32)
    , m_fixed_loop(60.0f)
    , m_begin_time(std::chrono::high_resolution_clock::now())
{
    LOG->set_level(spdlog::level::info);

    m_window.set_min_size({ 800, 600 });
    m_window.disable_cursor();

    auto resize_func = [&](nnm::Vector2i) {
        m_renderer.resize(m_window);
        m_world.resize(m_renderer.extent());
        m_ui_pipeline.resize();
        m_text_pipeline.resize();
        draw();
    };

    m_window.set_resize_callback(resize_func);

    std::invoke(resize_func, m_window.size());

    auto [fullscreen, msaa] = load_options();
    if (fullscreen) {
        m_window.fullscreen(true);
    }
    else {
        m_window.windowed();
    }
    m_renderer.set_msaa_samples(m_window, msaa);

    // ENetAddress address;
    // enet_address_set_host_ip(&address, "127.0.0.1");
    // address.port = c_server_port;
    // ENetPeer* peer = enet_host_connect(m_client, &address, 2, 0);
    // VV_REL_ASSERT(peer != nullptr, "[App] No available peers for initiating an ENet connection");
    // ENetEvent event;
    // if (enet_host_service(m_client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
    //     LOG->info("[App] Connected to server");
    // }
    // else {
    //     enet_peer_reset(peer);
    //     LOG->error("[App] Failed to connect to server;");
    // }
    // enet_host_flush(m_client);
}

App::~App()
{
    // enet_host_flush(m_client);
    // enet_host_destroy(m_client);
    // enet_deinitialize();
}

void App::draw()
{
    m_renderer.begin_frame(m_window);

    constexpr std::array sky_color { 142.0f / 255.0f, 186.0f / 255.0f, 1.0f, 1.0f };
    m_renderer.begin_render_pass_present(sky_color);

    m_world.draw();

    m_renderer.end_render_pass();

    m_renderer.end_frame(m_window);
}

void App::main_loop()
{
    while (!m_window.should_close() && !m_world.should_exit()) {
        // handle_networking();

        m_window.poll_events();

        m_fixed_loop.update(5, [&] { m_world.fixed_update(m_window); });

        m_world.update(m_window, m_fixed_loop.blend(), m_renderer);

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

        if (std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration_cast<std::chrono::microseconds>(end_time - m_begin_time).count() >= 1000000) {
            m_begin_time = std::chrono::high_resolution_clock::now();
            m_frame_count = m_current_frame_count;
            m_current_frame_count = 0;
        }
        m_current_frame_count++;
    }

    const Options options { .fullscreen = m_window.is_fullscreen(), .msaa = m_renderer.current_msaa_samples() };
    set_options(options);
}

// void App::handle_networking() const
// {
//     ENetEvent event;
//     std::string buffer;
//     while (enet_host_service(m_client, &event, 0) > 0) {
//         switch (event.type) {
//         case ENET_EVENT_TYPE_DISCONNECT:
//             LOG->info("[App] Disconnected from server");
//             break;
//         case ENET_EVENT_TYPE_RECEIVE:
//             buffer = std::string(event.packet->data, event.packet->data + event.packet->dataLength);
//             LOG->info("[App] Receieved packet: {}", buffer);
//             enet_packet_destroy(event.packet);
//             break;
//         default:
//             break;
//         }
//     }
// }

}
