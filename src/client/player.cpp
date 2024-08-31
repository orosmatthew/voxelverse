#include "player.hpp"

#include "common.hpp"
#include <cereal/archives/portable_binary.hpp>

#include "world_data.hpp"
#include <nnm/nnm.hpp>

Player::Player()
    : m_prev_pos(nnm::Vector3f(0, 0, 0))
    , m_friction(0.3f)
    , m_acceleration(0.035f)
    , m_max_speed(0.72f)
    , m_last_jump_time(std::chrono::steady_clock::now())
    , m_last_space_time(std::chrono::steady_clock::now())
    , m_is_flying(false)
    , m_save_loop(1.0f)
    , m_save(1024 * 1024, "player")
{
    if (std::optional<std::string> player_data = m_save.at<std::string>("pos"); player_data.has_value()) {
        std::stringstream data_stream(*player_data);
        cereal::PortableBinaryInputArchive archive_in(data_stream);
        archive_in(*this);
        m_pos = m_pos.translate({ 0, 0, 1 });
    }
}

void Player::update(const mve::Window& window, const bool capture_input)
{
    const nnm::Vector2f mouse_delta = capture_input ? window.mouse_delta() : nnm::Vector2f::zero();
    m_head_rotation.x += mouse_delta.x * 0.001f;
    m_head_rotation.y += mouse_delta.y * 0.001f;
    m_head_rotation.y = nnm::clamp(m_head_rotation.y, nnm::radians(0.1f), nnm::radians(179.9f));
    // if (m_head_rotation.y < nnm::radians(0.1f)) {
    //     m_head_rotation.y = 0.1f;
    // }
    // if (m_head_rotation.y > nnm::radians(179.9f)) {
    //     m_head_rotation.y = 179.9f;
    // }

    if (capture_input && window.is_key_pressed(mve::Key::space)) {
        if (const auto now = std::chrono::steady_clock::now();
            std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_space_time).count() <= 250) {
            if (m_is_flying) {
                m_is_flying = false;
            }
            else {
                m_is_flying = true;
            }
        }
        else {
            m_last_space_time = now;
        }
    }
    m_save_loop.update(1, [this] { save_pos(); });
}
void Player::fixed_update(const mve::Window& window, const WorldData& data, const bool capture_input)
{
    m_prev_pos = m_pos;
    const bool on_ground = is_on_ground(data);
    nnm::Vector3f dir;
    if (capture_input) {
        if (window.is_key_down(mve::Key::w)) {
            dir.y -= 1.0f;
        }
        if (window.is_key_down(mve::Key::s)) {
            dir.y += 1.0f;
        }
        if (window.is_key_down(mve::Key::a)) {
            dir.x -= 1.0f;
        }
        if (window.is_key_down(mve::Key::d)) {
            dir.x += 1.0f;
        }
        if (m_is_flying && window.is_key_down(mve::Key::space)) {
            dir.z += 1.0f;
        }
        if (m_is_flying && window.is_key_down(mve::Key::left_shift)) {
            dir.z -= 1.0f;
        }
    }
    dir = dir.rotate_axis_angle(nnm::Vector3f::axis_z(), m_head_rotation.x);
    if (!m_is_flying) {
        if (on_ground) {
            m_velocity -= m_velocity * m_friction * 0.9f;
        }
        else {
            m_velocity.x -= m_velocity.x * m_friction * 0.1f;
            m_velocity.y -= m_velocity.y * m_friction * 0.1f;
        }
        m_velocity.z -= 0.014f;
    }
    else {
        m_velocity -= m_velocity * m_friction * 0.35f;
    }

    if (!m_is_flying && on_ground) {
        if (capture_input && window.is_key_down(mve::Key::space)
            && std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - m_last_jump_time)
                    .count()
                >= 200) {
            m_velocity.z = 0.21f;
            m_last_jump_time = std::chrono::steady_clock::now();
        }
    }
    if (!m_is_flying) {
        if (on_ground && dir != nnm::Vector3f::zero()) {
            m_velocity += dir.normalize() * m_acceleration
                * nnm::clamp(m_max_speed - nnm::Vector2(m_velocity.x, m_velocity.y).length(), 0.0f, 1.0f);
        }
        else if (!on_ground && dir != nnm::Vector3f::zero()) {
            m_velocity
                += dir.normalize() * m_acceleration * 0.1f * nnm::clamp(m_max_speed - m_velocity.length(), 0.0f, 1.0f);
        }
    }
    else {
        if (capture_input && window.is_key_down(mve::Key::left_control)) {
            m_velocity
                += dir.normalize() * m_acceleration * nnm::clamp(30.0f - m_velocity.length(), 0.0f, 1.0f) * 10.0f;
        }
        else {
            m_velocity
                += dir.normalize() * m_acceleration * nnm::clamp(m_max_speed - m_velocity.length(), 0.0f, 1.0f) * 1.5f;
        }
    }

    m_velocity = move_and_slide(bounding_box(), m_pos, m_velocity, data);

    if (m_is_flying && is_on_ground(data)) {
        m_is_flying = false;
    }
}

nnm::Vector3f Player::move_and_slide(
    BoundingBox box, nnm::Vector3f& pos, const nnm::Vector3f velocity, const WorldData& data)
{
    auto detect_collision = [](const nnm::Vector3f vel, const BoundingBox& bbox, const WorldData& world_data) {
        const BoundingBox broadphase_box = swept_broadphase_box(vel, bbox);
        SweptBoundingBoxCollision min_collision { .time = 1.0f, .normal = nnm::Vector3f::zero() };
        const auto min_neighbor = nnm::Vector3i(broadphase_box.min.round());
        const nnm::Vector3i max_neighbor = nnm::Vector3i(broadphase_box.max.round()) + nnm::Vector3i(1, 1, 2);
        for_3d(min_neighbor, max_neighbor, [&](const nnm::Vector3i neighbor) {
            const nnm::Vector3i block_pos = neighbor - nnm::Vector3i(0, 0, 1);
            const std::optional<uint8_t> block = world_data.block_at(block_pos);
            if (const BoundingBox block_bb { { nnm::Vector3f(block_pos) - nnm::Vector3f::all(0.5f) },
                                             { nnm::Vector3f(block_pos) + nnm::Vector3f::all(0.5f) } };
                block.has_value() && block.value() != 0 && collides(block_bb, broadphase_box)) {
                if (const SweptBoundingBoxCollision collision = swept_bounding_box(vel, bbox, block_bb);
                    collision.time < min_collision.time) {
                    min_collision = collision;
                }
            }
        });
        return min_collision;
    };

    SweptBoundingBoxCollision collision = detect_collision(velocity, box, data);

    constexpr float collision_padding = 0.001f;

    pos = pos.translate(velocity * (collision.time - collision_padding));
    box.min += velocity * (collision.time - collision_padding);
    box.max += velocity * (collision.time - collision_padding);

    float remaining_time = 1.0f - collision.time;
    if (remaining_time == 0.0f) {
        return velocity;
    }
    nnm::Vector3 slide_vel = velocity * (nnm::Vector3(1.0f, 1.0f, 1.0f) - collision.normal.abs()) * remaining_time;

    collision = detect_collision(slide_vel, box, data);

    pos = pos.translate(slide_vel * (collision.time - collision_padding));
    box.min += velocity * (collision.time - collision_padding);
    box.max += velocity * (collision.time - collision_padding);

    remaining_time = 1.0f - collision.time;
    if (remaining_time == 0.0f) {
        return slide_vel;
    }
    slide_vel = slide_vel * (nnm::Vector3(1.0f, 1.0f, 1.0f) - collision.normal.abs()) * remaining_time;

    collision = detect_collision(slide_vel, box, data);

    pos = pos.translate(slide_vel * (collision.time - collision_padding));

    return slide_vel;
}

bool Player::is_on_ground(const WorldData& data) const
{
    BoundingBox bb = bounding_box();
    bb.min += nnm::Vector3(0.001f, 0.001f, -0.001f);
    bb.max += nnm::Vector3(-0.001f, -0.001f, -0.001f);
    const nnm::Vector3i block_pos = block_position();
    bool is_on_ground = false;
    for_2d({ -1, -1 }, { 2, 2 }, [&](const nnm::Vector2i neighbor) {
        const BoundingBox block_bb {
            { nnm::Vector3f(block_pos + nnm::Vector3i(neighbor.x, neighbor.y, -1)) - nnm::Vector3f::all(0.5f) },
            { nnm::Vector3f(block_pos + nnm::Vector3i(neighbor.x, neighbor.y, -1)) + nnm::Vector3f::all(0.5f) }
        };
        if (const std::optional<uint8_t> block = data.block_at(block_pos + nnm::Vector3i(neighbor.x, neighbor.y, -1));
            block.has_value() && block.value() != 0 && collides(bb, block_bb)) {
            is_on_ground = true;
        }
    });
    return is_on_ground;
}
Player::~Player()
{
    save_pos();
}
void Player::save_pos()
{
    m_save.insert<std::string, Player>("pos", *this);
}
