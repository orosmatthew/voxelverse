#include "app.hpp"

#include <chrono>

#include "FastNoiseLite.h"

#include "camera.hpp"
#include "chunk_mesh.hpp"
#include "logger.hpp"
#include "math/functions.hpp"
#include "math/matrix4.hpp"
#include "renderer.hpp"
#include "util/fixed_loop.hpp"
#include "window.hpp"
#include "world_renderer.hpp"

namespace app {

std::vector<mve::Vector3i> ray_blocks(mve::Vector3 start, mve::Vector3 end)
{
    mve::Vector3 delta = end - start;
    int step = mve::ceil(mve::max(mve::abs(delta.x), mve::max(mve::abs(delta.y), mve::abs(delta.z))));
    mve::Vector3 increment = delta / static_cast<float>(step);
    std::vector<mve::Vector3i> blocks;
    mve::Vector3 current = start;
    for (int i = 0; i < step; i++) {
        mve::Vector3i block { static_cast<int>(mve::round(current.x)),
                              static_cast<int>(mve::round(current.y)),
                              static_cast<int>(mve::round(current.z)) };
        blocks.push_back(block);
        current += increment;
    }
    return blocks;
}

void run()
{
    LOG->set_level(spdlog::level::warn);
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

    Camera camera;

    world_renderer.set_view(camera.view_matrix(1.0f));

    util::FixedLoop fixed_loop(60.0f);

    bool cursor_captured = true;

    while (!window.should_close()) {
        window.poll_events();

        fixed_loop.update(20, [&]() { camera.fixed_update(window); });

        if (cursor_captured) {
            camera.update(window);
        }

        mve::Matrix4 view = camera.view_matrix(fixed_loop.blend());

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

        if (window.is_mouse_button_pressed(mve::MouseButton::left)) {
            mve::Vector3i player_block_pos { static_cast<int>(mve::round(camera.position().x)),
                                             static_cast<int>(mve::round(camera.position().y)),
                                             static_cast<int>(mve::round(camera.position().z)) };
            std::optional<uint8_t> player_block = world_data.block_at(player_block_pos);
            if (player_block.has_value() && player_block.value() == 0) {
                world_data.set_block(player_block_pos, 1);
                world_renderer.add_data(
                    world_data.chunk_data_at(world_data.chunk_pos_from_block_pos(player_block_pos)), world_data);
            }
        }

        if (window.is_mouse_button_pressed(mve::MouseButton::right)) {
            //            mve::Vector3 end = pos + (thing * 10.0f);
            //            LOG->warn("start: {}, {}, {}", pos.x, pos.y, pos.z);
            //            LOG->warn("end: {}, {}, {}", end.x, end.y, end.z);
            //            std::vector<mve::Vector3i> blocks = ray_blocks(pos, end);
            //            for (mve::Vector3i block_pos : blocks) {
            //                std::optional<uint8_t> block = world_data.block_at(block_pos);
            //                if (!block.has_value() || block.value() != 0) {
            //                    continue;
            //                }
            //                world_data.set_block(block_pos, 1);
            //                world_renderer.add_data(
            //                    world_data.chunk_data_at(world_data.chunk_pos_from_block_pos(block_pos)), world_data);
            //            }
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
