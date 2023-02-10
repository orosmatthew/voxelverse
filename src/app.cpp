#include "app.hpp"

#include <chrono>

#include "FastNoiseLite.h"

#include "camera.hpp"
#include "chunk_mesh.hpp"
#include "logger.hpp"
#include "math/math.hpp"
#include "renderer.hpp"
#include "ui_renderer.hpp"
#include "util/fixed_loop.hpp"
#include "window.hpp"
#include "world_renderer.hpp"

namespace app {

std::vector<mve::Vector3i> ray_blocks(mve::Vector3 start, mve::Vector3 end)
{
    mve::Vector3 delta = end - start;
    int step = mve::ceil(mve::max(mve::abs(delta.x), mve::max(mve::abs(delta.y), mve::abs(delta.z))));
    mve::Vector3 increment = delta / static_cast<float>(step);
    std::set<mve::Vector3i> blocks_set;
    mve::Vector3 current = start;
    for (int i = 0; i < step; i++) {
        mve::Vector3i block { static_cast<int>(mve::round(current.x)),
                              static_cast<int>(mve::round(current.y)),
                              static_cast<int>(mve::round(current.z)) };
        blocks_set.insert(block);
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                for (int z = -1; z <= 1; z++) {
                    blocks_set.insert(block + mve::Vector3i(x, y, z));
                }
            }
        }
        current += increment;
    }
    std::vector<mve::Vector3i> blocks;
    blocks.reserve(blocks.size());
    for (mve::Vector3i block : blocks_set) {
        blocks.push_back(block);
    }
    std::sort(blocks.begin(), blocks.end(), [start](const mve::Vector3i& a, const mve::Vector3i& b) {
        return start.distance_squared_to(mve::Vector3(a)) < start.distance_squared_to(mve::Vector3(b));
    });
    return blocks;
}

struct Ray {
    mve::Vector3 position;
    mve::Vector3 direction;
};

struct BoundingBox {
    mve::Vector3 min;
    mve::Vector3 max;
};

struct RayCollision {
    bool hit;
    float distance;
    mve::Vector3 point;
    mve::Vector3 normal;
};

RayCollision ray_box_collision(Ray ray, const BoundingBox& box)
{
    bool inside = (ray.position.x > box.min.x) && (ray.position.x < box.max.x) && (ray.position.y > box.min.y)
        && (ray.position.y < box.max.y) && (ray.position.z > box.min.z) && (ray.position.z < box.max.z);

    if (inside) {
        ray.direction = -ray.direction;
    }

    std::array<float, 11> t = { 0 };

    t[8] = 1.0f / ray.direction.x;
    t[9] = 1.0f / ray.direction.y;
    t[10] = 1.0f / ray.direction.z;

    t[0] = (box.min.x - ray.position.x) * t[8];
    t[1] = (box.max.x - ray.position.x) * t[8];
    t[2] = (box.min.y - ray.position.y) * t[9];
    t[3] = (box.max.y - ray.position.y) * t[9];
    t[4] = (box.min.z - ray.position.z) * t[10];
    t[5] = (box.max.z - ray.position.z) * t[10];
    t[6] = mve::max(mve::max(mve::min(t[0], t[1]), mve::min(t[2], t[3])), mve::min(t[4], t[5]));
    t[7] = mve::min(mve::min(mve::max(t[0], t[1]), mve::max(t[2], t[3])), mve::max(t[4], t[5]));

    RayCollision collision = { 0 };
    collision.hit = !((t[7] < 0) || (t[6] > t[7]));
    collision.distance = t[6];
    collision.point = ray.position + (ray.direction * collision.distance);
    collision.normal = box.min.linear_interpolate(box.max, 0.5f);
    collision.normal = collision.point - collision.normal;
    collision.normal *= 2.01f;
    collision.normal /= (box.max - box.min);
    collision.normal.x = (float)((int)collision.normal.x);
    collision.normal.y = (float)((int)collision.normal.y);
    collision.normal.z = (float)((int)collision.normal.z);
    collision.normal = collision.normal.normalize();

    if (inside) {
        ray.direction = -ray.direction;
        collision.distance *= -1.0f;
        collision.normal = -collision.normal;
    }
    return collision;
}

void trigger_place_block(const Camera& camera, WorldData& world_data, WorldRenderer& world_renderer)
{
    std::set<mve::Vector3i> update_chunks;
    std::vector<mve::Vector3i> blocks = ray_blocks(camera.position(), camera.position() + (camera.direction() * 10.0f));
    Ray ray { camera.position(), camera.direction().normalize() };
    for (mve::Vector3i block_pos : blocks) {
        std::optional<uint8_t> block = world_data.block_at(block_pos);
        if (!block.has_value() || block.value() != 1) {
            continue;
        }
        BoundingBox bb { { mve::Vector3(block_pos) - mve::Vector3(0.5f, 0.5f, 0.5f) },
                         { mve::Vector3(block_pos) + mve::Vector3(0.5f, 0.5f, 0.5f) } };
        RayCollision collision = ray_box_collision(ray, bb);
        if (collision.hit) {
            mve::Vector3i place_pos { static_cast<int>(mve::round(block_pos.x + collision.normal.x)),
                                      static_cast<int>(mve::round(block_pos.y + collision.normal.y)),
                                      static_cast<int>(mve::round(block_pos.z + collision.normal.z)) };
            if (!world_data.block_at(place_pos).has_value() || world_data.block_at(place_pos).value() != 0) {
                break;
            }
            world_data.set_block(place_pos, 1);
            update_chunks.insert(WorldData::chunk_pos_from_block_pos(block_pos));
            std::array<mve::Vector3i, 6> surrounding
                = { mve::Vector3i(1, 0, 0),  mve::Vector3i(-1, 0, 0), mve::Vector3i(0, 1, 0),
                    mve::Vector3i(0, -1, 0), mve::Vector3i(0, 0, 1),  mve::Vector3i(0, 0, -1) };
            for (mve::Vector3i surround_pos : surrounding) {
                if (world_data.chunk_in_bounds(WorldData::chunk_pos_from_block_pos(block_pos) + surround_pos)) {
                    update_chunks.insert(WorldData::chunk_pos_from_block_pos(block_pos) + surround_pos);
                }
            }
            break;
        }
    }
    for (mve::Vector3i chunk_pos : update_chunks) {
        world_renderer.add_data(world_data.chunk_data_at(chunk_pos), world_data);
    }
}

void trigger_break_block(const Camera& camera, WorldData& world_data, WorldRenderer& world_renderer)
{
    std::set<mve::Vector3i> update_chunks;
    std::vector<mve::Vector3i> blocks = ray_blocks(camera.position(), camera.position() + (camera.direction() * 10.0f));
    Ray ray { camera.position(), camera.direction().normalize() };
    for (mve::Vector3i block_pos : blocks) {
        std::optional<uint8_t> block = world_data.block_at(block_pos);
        if (!block.has_value() || block.value() != 1) {
            continue;
        }
        BoundingBox bb { { mve::Vector3(block_pos) - mve::Vector3(0.5f, 0.5f, 0.5f) },
                         { mve::Vector3(block_pos) + mve::Vector3(0.5f, 0.5f, 0.5f) } };
        RayCollision collision = ray_box_collision(ray, bb);
        if (collision.hit) {
            mve::Vector3i local_pos = WorldData::block_world_to_local(block_pos);
            mve::Vector3i chunk_pos = WorldData::chunk_pos_from_block_pos(block_pos);
            world_data.set_block_local(chunk_pos, local_pos, 0);
            update_chunks.insert(chunk_pos);
            std::array<mve::Vector3i, 6> surrounding
                = { mve::Vector3i(1, 0, 0),  mve::Vector3i(-1, 0, 0), mve::Vector3i(0, 1, 0),
                    mve::Vector3i(0, -1, 0), mve::Vector3i(0, 0, 1),  mve::Vector3i(0, 0, -1) };
            for (mve::Vector3i surround_pos : surrounding) {
                if (world_data.chunk_in_bounds(chunk_pos + surround_pos)) {
                    update_chunks.insert(chunk_pos + surround_pos);
                }
            }
            break;
        }
    }
    for (mve::Vector3i chunk_pos : update_chunks) {
        world_renderer.add_data(world_data.chunk_data_at(chunk_pos), world_data);
    }
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

    UIRenderer ui_renderer(renderer);

    mve::Framebuffer world_framebuffer = renderer.create_framebuffer([&]() {
        ui_renderer.update_framebuffer_texture(
            world_framebuffer.texture(), renderer.framebuffer_size(world_framebuffer));
    });

    WorldGenerator world_generator(1);
    WorldData world_data(world_generator, { -32, -32, -4 }, { 32, 32, 4 });

    //    world_data.for_all_chunk_data([&](mve::Vector3i chunk_pos, const ChunkData& chunk_data) {
    //        world_renderer.add_data(chunk_data, world_data);
    //    });

    std::chrono::high_resolution_clock::time_point begin_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;

    auto resize_func = [&](mve::Vector2i new_size) {
        renderer.resize(window);
        world_renderer.resize();
        ui_renderer.resize();
    };

    window.set_resize_callback(resize_func);

    std::invoke(resize_func, window.size());

    Camera camera;

    world_renderer.set_view(camera.view_matrix(1.0f));

    util::FixedLoop fixed_loop(60.0f);

    bool cursor_captured = true;

    mve::Vector3i current_gen { -32, -32, -4 };

    while (!window.should_close()) {
        window.poll_events();

        fixed_loop.update(20, [&]() { camera.fixed_update(window); });

        if (cursor_captured) {
            camera.update(window);
        }

        if (current_gen != mve::Vector3i(32, 32, 4)) {
            for (int x = -32; x < 32; x++) {
                current_gen.x = x;
                if (world_data.chunk_in_bounds(current_gen)) {
                    world_renderer.add_data(world_data.chunk_data_at(current_gen), world_data);
                }
            }
            if (current_gen.y < 32) {
                current_gen.y++;
                current_gen.x = -32;
            }
            else if (current_gen.z < 4) {
                current_gen.z++;
                current_gen.y = -32;
                current_gen.x = -32;
            }
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
            trigger_break_block(camera, world_data, world_renderer);
        }

        if (window.is_mouse_button_pressed(mve::MouseButton::right)) {
            trigger_place_block(camera, world_data, world_renderer);
        }

        renderer.begin_frame(window);

        renderer.begin_render_pass_framebuffer(world_framebuffer);

        world_renderer.draw();

        renderer.end_render_pass_framebuffer(world_framebuffer);

        renderer.begin_render_pass_present();

        ui_renderer.draw();

        renderer.end_render_pass_present();

        renderer.end_frame(window);

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
