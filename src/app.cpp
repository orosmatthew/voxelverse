#include "app.hpp"

#include <chrono>

#include "FastNoiseLite.h"

#include "chunk_mesh.hpp"
#include "logger.hpp"
#include "math/functions.hpp"
#include "math/matrix4.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "util.hpp"
#include "util/fixed_loop.hpp"
#include "window.hpp"
#include "world_renderer.hpp"

namespace app {

std::vector<mve::Vector3i> ray_blocks(mve::Vector3 start, mve::Vector3 end)
{
    mve::Vector3 delta = end - start;
    int steps = mve::max(mve::abs(delta.x), mve::max(mve::abs(delta.y), mve::abs(delta.z)));
    if (steps == 0) {
        return { mve::Vector3i(mve::round(start.x), mve::round(start.y), mve::round(start.z)) };
    }
    mve::Vector3i increment = delta / steps;
    std::vector<mve::Vector3i> blocks;
    for (int i = 0; i <= steps; ++i) {
        mve::Vector3i block { static_cast<int>(mve::round(start.x + i * increment.x)),
                              static_cast<int>(mve::round(start.y + i * increment.y)),
                              static_cast<int>(mve::round(start.z + i * increment.z)) };
        blocks.push_back(block);
    }
    return blocks;
}

void run()
{
    LOG->set_level(spdlog::level::info);
    LOG->debug("Creating window");

    mve::Window window("Mini Vulkan Engine", mve::Vector2i(800, 600));

    window.set_min_size({ 800, 600 });

    window.disable_cursor();

    mve::Renderer renderer(window, "Vulkan Testing", 0, 0, 1);

    WorldRenderer world_renderer(renderer);

    WorldGenerator world_generator(1);
    WorldData world_data(world_generator, { -4, -4, -4 }, { 4, 4, 4 });

    world_data.for_all_chunk_data([&](mve::Vector3i chunk_pos, const ChunkData& chunk_data) {
        world_renderer.add_data(chunk_data, world_data);
    });

    std::chrono::high_resolution_clock::time_point begin_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;

    auto resize_func = [&](mve::Vector2i new_size) {
        renderer.resize(window);
        world_renderer.resize();
    };

    window.set_resize_callback(resize_func);

    std::invoke(resize_func, window.size());

    mve::Matrix4 model = mve::Matrix4().rotate(mve::Vector3(0.0f, 0.0f, 1.0f), mve::radians(90.0f));
    mve::Matrix4 prev_model = model;

    const float camera_acceleration = 0.03f;
    const float camera_speed = 0.3f;
    const float camera_friction = 0.1f;
    mve::Vector3 camera_pos(0.0f, 3.0f, 0.0f);
    mve::Vector3 camera_pos_prev = camera_pos;
    mve::Vector3 camera_front(0.0f, -1.0f, 0.0f);
    mve::Vector3 camera_up(0.0f, 0.0f, 1.0f);
    mve::Vector3 camera_velocity(0.0f);
    mve::Matrix4 view;
    float camera_yaw = 0.0f;
    float camera_pitch = 0.0f;
    view = mve::look_at(camera_pos, camera_pos + camera_front, camera_up);

    world_renderer.set_view(view);

    util::FixedLoop fixed_loop(60.0f);

    bool cursor_captured = true;

    while (!window.should_close()) {
        window.poll_events();

        fixed_loop.update(20, [&]() {
            mve::Vector3 dir(0.0f);
            if (window.is_key_down(mve::Key::w)) {
                dir.x += mve::cos(mve::radians(camera_yaw));
                dir.y += mve::sin(mve::radians(camera_yaw));
            }
            if (window.is_key_down(mve::Key::s)) {
                dir.x -= mve::cos(mve::radians(camera_yaw));
                dir.y -= mve::sin(mve::radians(camera_yaw));
            }
            if (window.is_key_down(mve::Key::a)) {
                dir.x += mve::cos(mve::radians(camera_yaw + 90.0f));
                dir.y += mve::sin(mve::radians(camera_yaw + 90.0f));
            }
            if (window.is_key_down(mve::Key::d)) {
                dir.x -= mve::cos(mve::radians(camera_yaw + 90.0f));
                dir.y -= mve::sin(mve::radians(camera_yaw + 90.0f));
            }
            if (window.is_key_down(mve::Key::space)) {
                dir.z += 1;
            }
            if (window.is_key_down(mve::Key::left_shift)) {
                dir.z -= 1;
            }
            camera_velocity -= (camera_velocity * camera_friction);
            camera_pos_prev = camera_pos;
            if (dir != mve::Vector3(0.0f)) {
                camera_velocity += dir.normalize() * camera_acceleration;
            }
            if (camera_velocity.length() > camera_speed) {
                camera_velocity = camera_velocity.normalize() * camera_speed;
            }
            camera_pos += camera_velocity;

            prev_model = model;
            if (window.is_key_down(mve::Key::left)) {
                model = model.rotate(mve::Vector3(0.0f, 0.0f, 1.0f), mve::radians(1.0f));
            }
            if (window.is_key_down(mve::Key::right)) {
                model = model.rotate(mve::Vector3(0.0f, 0.0f, 1.0f), mve::radians(-1.0f));
            }
        });

        if (cursor_captured) {
            mve::Vector2 mouse_delta = window.mouse_delta();
            camera_yaw -= mouse_delta.x * 0.1f;
            camera_pitch -= mouse_delta.y * 0.1f;
        }

        camera_pitch = mve::clamp(camera_pitch, -89.0f, 89.0f);
        mve::Vector3 direction;
        direction.x = mve::cos(mve::radians(camera_yaw)) * mve::cos(mve::radians(camera_pitch));
        direction.y = mve::sin(mve::radians(camera_yaw)) * mve::cos(mve::radians(camera_pitch));
        direction.z = mve::sin(mve::radians(camera_pitch));
        camera_front = direction.normalize();

        mve::Vector3 pos = camera_pos_prev.linear_interpolate(camera_pos, fixed_loop.blend());
        view = mve::look_at(pos, pos + camera_front, camera_up);

        world_renderer.set_view(view);
        if (window.is_key_pressed(mve::Key::escape)) {
            break;
        }

        if (window.is_key_pressed(mve::Key::f)) {
            if (!window.is_fullscreen()) {
                window.fullscreen(true);
            }
            else {
                window.windowed();
            }
        }

        if (window.is_key_pressed(mve::Key::c)) {
            if (cursor_captured) {
                window.enable_cursor();
                cursor_captured = false;
            }
            else {
                window.disable_cursor();
                cursor_captured = true;
            }
        }

        renderer.begin(window);

        world_renderer.draw();

        renderer.end(window);

        std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time).count() >= 1000000) {
            begin_time = std::chrono::high_resolution_clock::now();
            LOG->info("Framerate: {}", frame_count);
            frame_count = 0;
        }

        frame_count++;
    }
}
}
