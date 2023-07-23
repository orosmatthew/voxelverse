#include "world.hpp"

#include "chunk_data.hpp"
#include "common.hpp"
#include "logger.hpp"
#include "ui_pipeline.hpp"

World::World(mve::Renderer& renderer, UIPipeline& ui_pipeline, TextPipeline& text_pipeline, int render_distance)
    : m_world_renderer(renderer)
    , m_world_generator(1)
    , m_render_distance(render_distance)
    , m_hud(ui_pipeline, text_pipeline)
    , m_pause_menu(ui_pipeline, text_pipeline)
    , m_last_place_time(std::chrono::steady_clock::now())
    , m_last_break_time(std::chrono::steady_clock::now())
    , m_focus(FocusState::world)
    , m_should_exit(false)
{
    m_hud.update_debug_gpu_name(renderer.gpu_name());
    m_chunk_controller.set_mesh_updates_per_frame(2).set_render_distance(render_distance);
}

void World::fixed_update(const mve::Window& window)
{
    m_player.fixed_update(window, m_world_data, m_focus == FocusState::world);
}

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
    std::copy(blocks_set.cbegin(), blocks_set.cend(), std::back_inserter(blocks));
    std::sort(blocks.begin(), blocks.end(), [start](const mve::Vector3i& a, const mve::Vector3i& b) {
        return start.distance_sqrd_to(mve::Vector3(a)) < start.distance_sqrd_to(mve::Vector3(b));
    });
    return blocks;
}

struct Ray {
    mve::Vector3 position;
    mve::Vector3 direction;
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
        collision.distance *= -1.0f;
        collision.normal = -collision.normal;
    }
    return collision;
}

void trigger_place_block(const Player& camera, WorldData& world_data, WorldRenderer& world_renderer, uint8_t block_type)
{
    std::set<mve::Vector3i> update_chunks;
    std::vector<mve::Vector3i> blocks = ray_blocks(camera.position(), camera.position() + (camera.direction() * 10.0f));
    Ray ray { camera.position(), camera.direction().normalize() };
    for (mve::Vector3i block_pos : blocks) {
        std::optional<uint8_t> block = world_data.block_at(block_pos);
        if (!block.has_value() || block.value() == 0) {
            continue;
        }
        BoundingBox bb { { mve::Vector3(block_pos) - mve::Vector3(0.5f, 0.5f, 0.5f) },
                         { mve::Vector3(block_pos) + mve::Vector3(0.5f, 0.5f, 0.5f) } };
        RayCollision collision = ray_box_collision(ray, bb);
        if (collision.hit) {
            mve::Vector3i place_pos { static_cast<int>(mve::round(block_pos.x + collision.normal.x)),
                                      static_cast<int>(mve::round(block_pos.y + collision.normal.y)),
                                      static_cast<int>(mve::round(block_pos.z + collision.normal.z)) };
            BoundingBox player_box = camera.bounding_box();
            BoundingBox broadphase_box = swept_broadphase_box(camera.velocity(), player_box);
            BoundingBox place_bb = { { mve::Vector3(place_pos) - mve::Vector3(0.5f) },
                                     { mve::Vector3(place_pos) + mve::Vector3(0.5f) } };
            if (collides(broadphase_box, place_bb)) {
                break;
            }
            if (!world_data.block_at(place_pos).has_value() || world_data.block_at(place_pos).value() != 0) {
                break;
            }
            world_data.set_block(place_pos, block_type);

            for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](const mve::Vector3i& adj_chunk) {
                world_data.push_chunk_lighting_update(WorldData::chunk_pos_from_block_pos(place_pos) + adj_chunk);
            });
            //            world_data.process_chunk_lighting_updates();

            update_chunks.insert(WorldData::chunk_pos_from_block_pos(block_pos));
            // TODO: Check chunks only on edges
            for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](const mve::Vector3i& surround_pos) {
                if (world_data.contains_chunk(WorldData::chunk_pos_from_block_pos(block_pos) + surround_pos)) {
                    update_chunks.insert(WorldData::chunk_pos_from_block_pos(block_pos) + surround_pos);
                }
            });
            break;
        }
    }
    for (mve::Vector3i chunk_pos : update_chunks) {
        world_renderer.push_mesh_update(chunk_pos);
    }
    world_renderer.process_mesh_updates(world_data);
}

void trigger_break_block(const Player& camera, WorldData& world_data, WorldRenderer& world_renderer)
{
    std::set<mve::Vector3i> update_chunks;
    std::vector<mve::Vector3i> blocks = ray_blocks(camera.position(), camera.position() + (camera.direction() * 10.0f));
    Ray ray { camera.position(), camera.direction().normalize() };
    for (mve::Vector3i block_pos : blocks) {
        std::optional<uint8_t> block = world_data.block_at(block_pos);
        if (!block.has_value() || block.value() == 0) {
            continue;
        }
        BoundingBox bb { { mve::Vector3(block_pos) - mve::Vector3(0.5f, 0.5f, 0.5f) },
                         { mve::Vector3(block_pos) + mve::Vector3(0.5f, 0.5f, 0.5f) } };
        RayCollision collision = ray_box_collision(ray, bb);
        if (collision.hit) {
            mve::Vector3i local_pos = WorldData::block_world_to_local(block_pos);
            mve::Vector3i chunk_pos = WorldData::chunk_pos_from_block_pos(block_pos);

            for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](const mve::Vector3i& adj_chunk) {
                world_data.push_chunk_lighting_update(WorldData::chunk_pos_from_block_pos(block_pos) + adj_chunk);
            });

            world_data.set_block_local(chunk_pos, local_pos, 0);
            //            world_data.process_chunk_lighting_updates();
            update_chunks.insert(chunk_pos);
            for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](const mve::Vector3i& surround_pos) {
                if (world_data.contains_chunk(WorldData::chunk_pos_from_block_pos(block_pos) + surround_pos)) {
                    update_chunks.insert(WorldData::chunk_pos_from_block_pos(block_pos) + surround_pos);
                }
            });
            break;
        }
    }
    for (mve::Vector3i chunk_pos : update_chunks) {
        world_renderer.push_mesh_update(chunk_pos);
    }
    world_renderer.process_mesh_updates(world_data);
}

void World::update(mve::Window& window, float blend)
{
    if (window.is_key_pressed(mve::Key::f3)) {
        m_hud.toggle_debug();
    }
    if (m_hud.is_debug_enabled()) {
        m_hud.update_debug_player_block_pos(m_player.block_position());
    }

    m_player.update(window, m_focus == FocusState::world);
    m_world_renderer.set_view(m_player.view_matrix(blend));

    switch (m_focus) {
    case FocusState::world:
        update_world(window, blend);
        break;
    case FocusState::console:
        m_hud.update_console(window);
        if (window.is_key_pressed(mve::Key::escape)) {
            m_focus = FocusState::world;
            window.disable_cursor();
            m_hud.disable_console_cursor();
        }
        break;
    case FocusState::pause:
        if (window.is_key_pressed(mve::Key::escape)) {
            m_focus = FocusState::world;
            window.disable_cursor();
        }
        m_pause_menu.update(window);
        if (m_pause_menu.exit_pressed()) {
            m_should_exit = true;
        }
        if (m_pause_menu.back_pressed()) {
            m_focus = FocusState::world;
            window.disable_cursor();
            m_last_break_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
        }
        if (m_pause_menu.fullscreen_toggled()) {
            window.is_fullscreen() ? window.windowed() : window.fullscreen(true);
        }
        break;
    }

    std::vector<mve::Vector3i> blocks
        = ray_blocks(m_player.position(), m_player.position() + (m_player.direction() * 10.0f));
    Ray ray { m_player.position(), m_player.direction().normalize() };
    m_world_renderer.hide_selection();
    for (mve::Vector3i block_pos : blocks) {
        std::optional<uint8_t> block = m_world_data.block_at(block_pos);
        if (!block.has_value() || block.value() == 0) {
            continue;
        }
        BoundingBox bb { { mve::Vector3(block_pos) - mve::Vector3(0.5f, 0.5f, 0.5f) },
                         { mve::Vector3(block_pos) + mve::Vector3(0.5f, 0.5f, 0.5f) } };
        RayCollision collision = ray_box_collision(ray, bb);
        if (collision.hit) {
            m_world_renderer.show_selection();
            m_world_renderer.set_selection_position(mve::Vector3(block_pos));
            break;
        }
    }

    m_chunk_controller.update(
        m_world_data, m_world_generator, m_world_renderer, chunk_pos_from_block_pos(m_player.block_position()));
}

void World::resize(mve::Vector2i extent)
{
    m_world_renderer.resize();
    m_hud.resize(extent);
    m_pause_menu.resize(extent);
}
void World::draw()
{
    m_world_renderer.draw(m_player);
    m_hud.draw();
    if (m_focus == FocusState::pause) {
        m_pause_menu.draw();
    }
}
mve::Vector3i World::player_block_pos() const
{
    return m_player.block_position();
}
mve::Vector3i World::player_chunk_pos() const
{
    return WorldData::chunk_pos_from_block_pos(m_player.block_position());
}
std::optional<const ChunkData*> World::chunk_data_at(mve::Vector3i chunk_pos) const
{
    if (m_world_data.contains_chunk(chunk_pos)) {
        return &m_world_data.chunk_data_at(chunk_pos);
    }
    else {
        return {};
    }
}
void World::update_world(mve::Window& window, float blend)
{
    if (window.is_key_pressed(mve::Key::escape)) {
        m_focus = FocusState::pause;
        window.enable_cursor();
    }
    auto now = std::chrono::steady_clock::now();
    if (window.is_mouse_button_pressed(mve::MouseButton::left)) {
        trigger_break_block(m_player, m_world_data, m_world_renderer);
        m_last_break_time = now;
    }
    if (window.is_mouse_button_down(mve::MouseButton::left)) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_break_time).count() > 200) {
            trigger_break_block(m_player, m_world_data, m_world_renderer);
            m_last_break_time = now;
        }
    }

    if (window.is_mouse_button_pressed(mve::MouseButton::right)) {
        if (m_hud.hotbar().item_at(m_hud.hotbar().select_pos()).has_value()) {
            trigger_place_block(
                m_player, m_world_data, m_world_renderer, *m_hud.hotbar().item_at(m_hud.hotbar().select_pos()));
            m_last_place_time = now;
        }
    }
    if (window.is_mouse_button_down(mve::MouseButton::right)) {
        if (m_hud.hotbar().item_at(m_hud.hotbar().select_pos()).has_value()) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_place_time).count() > 200) {
                trigger_place_block(
                    m_player, m_world_data, m_world_renderer, *m_hud.hotbar().item_at(m_hud.hotbar().select_pos()));
                m_last_place_time = now;
            }
        }
    }

    mve::Vector2 scroll = window.mouse_scroll();
    int scroll_y = static_cast<int>(scroll.y);
    if (scroll_y != 0) {
        for (int i = 0; i < mve::abs(scroll_y); i++) {
            if (scroll_y < 0) {
                if (m_hud.hotbar().select_pos() + 1 > 8) {
                    m_hud.hotbar().update_hotbar_select(0);
                }
                else {
                    m_hud.hotbar().update_hotbar_select(m_hud.hotbar().select_pos() + 1);
                }
            }
            else {
                if (m_hud.hotbar().select_pos() - 1 < 0) {
                    m_hud.hotbar().update_hotbar_select(8);
                }
                else {
                    m_hud.hotbar().update_hotbar_select(m_hud.hotbar().select_pos() - 1);
                }
            }
        }
    }

    if (window.is_key_pressed(mve::Key::one)) {
        m_hud.hotbar().update_hotbar_select(0);
    }
    if (window.is_key_pressed(mve::Key::two)) {
        m_hud.hotbar().update_hotbar_select(1);
    }
    if (window.is_key_pressed(mve::Key::three)) {
        m_hud.hotbar().update_hotbar_select(2);
    }
    if (window.is_key_pressed(mve::Key::four)) {
        m_hud.hotbar().update_hotbar_select(3);
    }
    if (window.is_key_pressed(mve::Key::five)) {
        m_hud.hotbar().update_hotbar_select(4);
    }
    if (window.is_key_pressed(mve::Key::six)) {
        m_hud.hotbar().update_hotbar_select(5);
    }
    if (window.is_key_pressed(mve::Key::seven)) {
        m_hud.hotbar().update_hotbar_select(6);
    }
    if (window.is_key_pressed(mve::Key::eight)) {
        m_hud.hotbar().update_hotbar_select(7);
    }
    if (window.is_key_pressed(mve::Key::nine)) {
        m_hud.hotbar().update_hotbar_select(8);
    }
    if (window.is_key_pressed(mve::Key::t)) {
        m_focus = FocusState::console;
        window.enable_cursor();
        m_hud.enable_console_cursor();
    }
}
