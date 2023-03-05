#include "world.hpp"

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <lz4hc.h>

#include "common.hpp"
#include "logger.hpp"
#include "ui_renderer.hpp"

World::World(mve::Window& window, mve::Renderer& renderer, UIRenderer& ui_renderer, int render_distance)
    : m_window(&window)
    , m_renderer(&renderer)
    , m_ui_renderer(&ui_renderer)
    , m_world_renderer(renderer)
    , m_world_generator(1)
    , m_mesh_updates_per_frame(2)
    , m_render_distance(render_distance)
{
    m_hotbar_blocks.insert({ 0, 1 });
    m_hotbar_blocks.insert({ 1, 2 });
    m_hotbar_blocks.insert({ 2, 3 });
    m_hotbar_blocks.insert({ 3, 4 });
    m_hotbar_blocks.insert({ 4, 5 });
    m_hotbar_blocks.insert({ 5, 6 });
    m_hotbar_blocks.insert({ 6, 7 });
    m_hotbar_blocks.insert({ 7, 8 });
    m_hotbar_blocks.insert({ 8, 9 });
    for (auto& [pos, block] : m_hotbar_blocks) {
        m_ui_renderer->set_hotbar_block(pos, block);
    }
}

void World::fixed_update()
{
    m_player.fixed_update(*m_window, m_world_data);
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
            update_chunks.insert(WorldData::chunk_pos_from_block_pos(block_pos));
            std::array<mve::Vector3i, 6> surrounding
                = { mve::Vector3i(1, 0, 0),  mve::Vector3i(-1, 0, 0), mve::Vector3i(0, 1, 0),
                    mve::Vector3i(0, -1, 0), mve::Vector3i(0, 0, 1),  mve::Vector3i(0, 0, -1) };
            for (mve::Vector3i surround_pos : surrounding) {
                if (world_data.contains_chunk(WorldData::chunk_pos_from_block_pos(block_pos) + surround_pos)) {
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
            world_data.set_block_local(chunk_pos, local_pos, 0);
            update_chunks.insert(chunk_pos);
            std::array<mve::Vector3i, 6> surrounding
                = { mve::Vector3i(1, 0, 0),  mve::Vector3i(-1, 0, 0), mve::Vector3i(0, 1, 0),
                    mve::Vector3i(0, -1, 0), mve::Vector3i(0, 0, 1),  mve::Vector3i(0, 0, -1) };
            for (mve::Vector3i surround_pos : surrounding) {
                if (world_data.contains_chunk(chunk_pos + surround_pos)) {
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

void World::update(bool mouse_captured, float blend)
{
    if (mouse_captured) {
        m_player.update(*m_window);
    }

    m_world_renderer.set_view(m_player.view_matrix(blend));

    if (m_window->is_mouse_button_pressed(mve::MouseButton::left)) {
        trigger_break_block(m_player, m_world_data, m_world_renderer);
    }

    if (m_window->is_mouse_button_pressed(mve::MouseButton::right)) {
        if (m_hotbar_blocks.contains(m_current_hotbar_select)) {
            trigger_place_block(m_player, m_world_data, m_world_renderer, m_hotbar_blocks.at(m_current_hotbar_select));
        }
    }

    mve::Vector2 scroll = m_window->mouse_scroll();
    int scroll_y = static_cast<int>(scroll.y);
    if (scroll_y != 0) {
        for (int i = 0; i < mve::abs(scroll_y); i++) {
            if (scroll_y < 0) {
                if (m_current_hotbar_select + 1 > 8) {
                    m_current_hotbar_select = 0;
                }
                else {
                    m_current_hotbar_select++;
                }
            }
            else {
                if (m_current_hotbar_select - 1 < 0) {
                    m_current_hotbar_select = 8;
                }
                else {
                    m_current_hotbar_select--;
                }
            }
        }
        m_ui_renderer->set_hotbar_select(m_current_hotbar_select);
    }

    if (m_window->is_key_pressed(mve::Key::one)) {
        m_current_hotbar_select = 0;
        m_ui_renderer->set_hotbar_select(0);
    }
    if (m_window->is_key_pressed(mve::Key::two)) {
        m_current_hotbar_select = 1;
        m_ui_renderer->set_hotbar_select(1);
    }
    if (m_window->is_key_pressed(mve::Key::three)) {
        m_current_hotbar_select = 2;
        m_ui_renderer->set_hotbar_select(2);
    }
    if (m_window->is_key_pressed(mve::Key::four)) {
        m_current_hotbar_select = 3;
        m_ui_renderer->set_hotbar_select(3);
    }
    if (m_window->is_key_pressed(mve::Key::five)) {
        m_current_hotbar_select = 4;
        m_ui_renderer->set_hotbar_select(4);
    }
    if (m_window->is_key_pressed(mve::Key::six)) {
        m_current_hotbar_select = 5;
        m_ui_renderer->set_hotbar_select(5);
    }
    if (m_window->is_key_pressed(mve::Key::seven)) {
        m_current_hotbar_select = 6;
        m_ui_renderer->set_hotbar_select(6);
    }
    if (m_window->is_key_pressed(mve::Key::eight)) {
        m_current_hotbar_select = 7;
        m_ui_renderer->set_hotbar_select(7);
    }
    if (m_window->is_key_pressed(mve::Key::nine)) {
        m_current_hotbar_select = 8;
        m_ui_renderer->set_hotbar_select(8);
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
            m_world_renderer.set_selection_position(block_pos);
            break;
        }
    }

    // Chunk updates
    mve::Vector3i current_player_chunk_3d = WorldData::chunk_pos_from_block_pos(m_player.block_position());
    mve::Vector2i current_player_chunk = { current_player_chunk_3d.x, current_player_chunk_3d.y };
    if (current_player_chunk != m_player_chunk) {
        m_player_chunk = current_player_chunk;
        m_sorted_chunks.clear();
        m_sorted_chunks.reserve(m_chunk_states.size());
        for (auto& [pos, data] : m_chunk_states) {
            if (mve::abs(mve::sqrd(pos.x - m_player_chunk.x) + mve::sqrd(pos.y - m_player_chunk.y))
                > mve::sqrd(m_render_distance)) {
                data.should_delete = true;
            }
        }
        for_2d(
            mve::Vector2i(-m_render_distance, -m_render_distance) + mve::Vector2i(m_player_chunk.x, m_player_chunk.y),
            mve::Vector2i(m_render_distance, m_render_distance) + mve::Vector2i(m_player_chunk.x, m_player_chunk.y),
            [&](mve::Vector2i pos) {
                if (mve::abs(mve::sqrd(pos.x - m_player_chunk.x) + mve::sqrd(pos.y - m_player_chunk.y))
                    <= mve::sqrd(m_render_distance)) {
                    if (!m_chunk_states.contains(pos)) {
                        m_chunk_states[pos] = {};
                    }
                    else {
                        m_chunk_states[pos].should_delete = false;
                    }
                }
            });
        for (auto& [pos, data] : m_chunk_states) {
            m_sorted_chunks.push_back(pos);
        }
        std::sort(m_sorted_chunks.begin(), m_sorted_chunks.end(), [&](const mve::Vector2i& a, const mve::Vector2i& b) {
            return mve::distance_sqrd(a, mve::Vector2i(m_player_chunk.x, m_player_chunk.y))
                < mve::distance_sqrd(b, mve::Vector2i(m_player_chunk.x, m_player_chunk.y));
        });
    }

    int chunk_count = 0;
    for (mve::Vector2i pos : m_sorted_chunks) {
        if (!m_chunk_states.at(pos).has_data) {
            bool loaded = false;
            for (int h = -10; h < 10; h++) {
                if (m_world_data.try_load_chunk_from_save({ pos.x, pos.y, h })) {
                    loaded = true;
                }
            }
            if (!loaded) {
                std::array<ChunkData*, 20> chunk_datas;
                for (int h = -10; h < 10; h++) {
                    m_world_data.create_chunk({ pos.x, pos.y, h });
                    chunk_datas[h + 10] = &(m_world_data.chunk_data_at({ pos.x, pos.y, h }));
                }
                m_world_generator.generate_chunks(chunk_datas, pos);
            }
            for_2d({ -1, -1 }, { 2, 2 }, [&](mve::Vector2i neighbor) {
                if (neighbor != mve::Vector2i(0, 0)) {
                    m_chunk_states[pos + neighbor].neighbors++;
                    if (m_chunk_states[pos + neighbor].has_data && m_chunk_states[pos + neighbor].neighbors == 8) {
                        m_chunk_states[pos + neighbor].can_mesh = true;
                    }
                }
            });
            m_chunk_states[pos].has_data = true;
            if (m_chunk_states[pos].neighbors == 8) {
                m_chunk_states[pos].can_mesh = true;
            }
            chunk_count++;
        }
        if (!m_chunk_states.at(pos).has_mesh && m_chunk_states.at(pos).can_mesh) {

            for (int h = -10; h < 10; h++) {
                m_world_renderer.add_data(m_world_data.chunk_data_at({ pos.x, pos.y, h }), m_world_data);
            }
            m_chunk_states[pos].has_mesh = true;
            chunk_count++;
        }
        if (chunk_count > m_mesh_updates_per_frame) {
            break;
        }
    }

    chunk_count = 0;
    for (int i = m_sorted_chunks.size() - 1; i >= 0; i--) {
        const mve::Vector2i pos = m_sorted_chunks[i];
        if (!m_chunk_states.at(pos).should_delete) {
            continue;
        }
        auto it = m_sorted_chunks.begin();
        while (it != m_sorted_chunks.end()) {
            if (*it == pos) {
                it = m_sorted_chunks.erase(it);
            }
            else {
                it++;
            }
        }
        if (m_chunk_states.at(pos).has_data) {
            for_2d({ -1, -1 }, { 2, 2 }, [&](mve::Vector2i neighbor) {
                if (neighbor == mve::Vector2i(0, 0)) {
                    return;
                }
                m_chunk_states[pos + neighbor].neighbors--;
            });
        }
        for (int h = -10; h < 10; h++) {
            if (m_chunk_states.at(pos).has_data) {
                m_world_data.remove_chunk({ pos.x, pos.y, h });
            }
            if (m_chunk_states.at(pos).has_mesh) {
                m_world_renderer.remove_data({ pos.x, pos.y, h });
            }
        }
        if (m_chunk_states.at(pos).has_mesh) {
            chunk_count++;
        }
        m_chunk_states.at(pos).has_data = false;
        m_chunk_states.at(pos).has_mesh = false;
        m_chunk_states.at(pos).can_mesh = false;
        m_chunk_states.at(pos).should_delete = true;
        if (m_chunk_states.at(pos).neighbors == 0) {
            m_chunk_states.erase(pos);
        }
        if (chunk_count > m_mesh_updates_per_frame) {
            break;
        }
    }
}

void World::resize()
{
    m_world_renderer.resize();
}
void World::draw()
{
    m_world_renderer.draw(m_player);
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
