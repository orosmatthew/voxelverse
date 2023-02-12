#include "app.hpp"

#include "logger.hpp"

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
        return start.distance_sqrd_to(mve::Vector3(a)) < start.distance_sqrd_to(mve::Vector3(b));
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

App::App()
    : m_window("Mini Vulkan Engine", mve::Vector2i(800, 600))
    , m_renderer(m_window, "Vulkan Testing", 0, 0, 1)
    , m_ui_renderer(m_renderer)
    , m_world_framebuffer(m_renderer.create_framebuffer([this]() {
        m_ui_renderer.update_framebuffer_texture(
            m_world_framebuffer.texture(), m_renderer.framebuffer_size(m_world_framebuffer));
    }))
    , m_fixed_loop(60.0f)
    , m_cursor_captured(true)
    , m_world_renderer(m_renderer)
    , m_mesh_updates_per_frame(32)
    , m_printed(false)
    , m_begin_time(std::chrono::high_resolution_clock::now())
    , m_frame_count(0)
    , m_camera()
    , m_world_data()
    , m_chunk_mesh_queue()
{
    LOG->set_level(spdlog::level::info);
    m_window.set_min_size({ 800, 600 });
    m_window.disable_cursor();

    WorldGenerator world_generator(1);
    m_world_data.generate(world_generator, { -32, -32, -4 }, { 32, 32, 4 });

    for_3d({ -32, -32, -4 }, { 32, 32, 4 }, [&](mve::Vector3i pos) { m_chunk_mesh_queue.push_back(pos); });

    std::sort(m_chunk_mesh_queue.begin(), m_chunk_mesh_queue.end(), [](const mve::Vector3i& a, const mve::Vector3i& b) {
        return mve::Vector3(a).distance_sqrd_to(mve::Vector3(0)) > mve::Vector3(b).distance_sqrd_to(mve::Vector3(0));
    });

    auto resize_func = [&](mve::Vector2i new_size) {
        m_renderer.resize(m_window);
        m_world_renderer.resize();
        m_ui_renderer.resize();
    };

    m_window.set_resize_callback(resize_func);

    std::invoke(resize_func, m_window.size());

    m_world_renderer.set_view(m_camera.view_matrix(1.0f));
}

void App::main_loop()
{
    while (!m_window.should_close()) {
        m_window.poll_events();

        m_fixed_loop.update(20, [&]() { m_camera.fixed_update(m_window); });

        if (m_cursor_captured) {
            m_camera.update(m_window);
        }

        int count = 0;
        while (!m_chunk_mesh_queue.empty()) {
            if (m_world_data.chunk_in_bounds(m_chunk_mesh_queue.back())) {
                m_world_renderer.add_data(m_world_data.chunk_data_at(m_chunk_mesh_queue.back()), m_world_data);
            }
            m_chunk_mesh_queue.pop_back();
            count++;
            if (count > m_mesh_updates_per_frame) {
                break;
            }
        }

        if (m_chunk_mesh_queue.empty() && !m_printed) {
            LOG->info("Done loading chunks");
            m_printed = true;
        }

        mve::Matrix4 view = m_camera.view_matrix(m_fixed_loop.blend());

        m_world_renderer.set_view(view);
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

        if (m_window.is_mouse_button_pressed(mve::MouseButton::left)) {
            trigger_break_block(m_camera, m_world_data, m_world_renderer);
        }

        if (m_window.is_mouse_button_pressed(mve::MouseButton::right)) {
            trigger_place_block(m_camera, m_world_data, m_world_renderer);
        }

        std::vector<mve::Vector3i> blocks
            = ray_blocks(m_camera.position(), m_camera.position() + (m_camera.direction() * 10.0f));
        Ray ray { m_camera.position(), m_camera.direction().normalize() };
        m_world_renderer.hide_selection();
        for (mve::Vector3i block_pos : blocks) {
            std::optional<uint8_t> block = m_world_data.block_at(block_pos);
            if (!block.has_value() || block.value() != 1) {
                continue;
            }
            BoundingBox bb { { mve::Vector3(block_pos) - mve::Vector3(0.5f, 0.5f, 0.5f) },
                             { mve::Vector3(block_pos) + mve::Vector3(0.5f, 0.5f, 0.5f) } };
            RayCollision collision = ray_box_collision(ray, bb);
            if (collision.hit) {
                m_world_renderer.show_selection();
                m_world_renderer.set_selection_position(block_pos);
                break;
            }
        }

        m_renderer.begin_frame(m_window);

        m_renderer.begin_render_pass_framebuffer(m_world_framebuffer);

        m_world_renderer.draw(m_camera);

        m_renderer.end_render_pass_framebuffer(m_world_framebuffer);

        m_renderer.begin_render_pass_present();

        m_ui_renderer.draw();

        m_renderer.end_render_pass_present();

        m_renderer.end_frame(m_window);

        std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::microseconds>(end_time - m_begin_time).count() >= 1000000) {
            m_begin_time = std::chrono::high_resolution_clock::now();
            LOG->info("Framerate: {}", m_frame_count);
            m_frame_count = 0;
        }

        m_frame_count++;
    }
}
}
