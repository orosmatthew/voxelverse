#include "player.hpp"

#include <limits>

#include "common.hpp"
#include "mve/math/math.hpp"
#include "world_data.hpp"

Player::Player()
    : m_body_transform(mve::Matrix4::identity())
    , m_head_transform(mve::Matrix4::identity())
    , m_prev_pos(mve::Vector3(0, 0, 0))
    , m_friction(0.1f)
    , m_acceleration(0.03f)
    , m_max_speed(0.3f)
{
}

void Player::update(const mve::Window& window)
{
    mve::Vector2 mouse_delta = window.mouse_delta();
    m_body_transform = m_body_transform.rotate_local({ 0, 0, 1 }, -mouse_delta.x * 0.001f);
    m_head_transform = m_head_transform.rotate_local({ 1, 0, 0 }, -mouse_delta.y * 0.001f);
    mve::Vector3 head_euler = m_head_transform.euler();
    if (head_euler.x < 0.0f) {
        if (mve::abs(head_euler.x) < mve::abs(head_euler.x + mve::pi)) {
            head_euler.x = 0.0f;
        }
        else {
            head_euler.x = 180.0f;
        }
    }
    if (head_euler.x < mve::radians(0.1f)) {
        m_head_transform = mve::Matrix4::from_basis_translation(
            mve::Matrix3::from_euler({ mve::radians(0.1f), 0, 0 }), m_head_transform.translation());
    }
    if (head_euler.x > mve::radians(179.9f)) {
        m_head_transform = mve::Matrix4::from_basis_translation(
            mve::Matrix3::from_euler({ mve::radians(179.9f), 0, 0 }), m_head_transform.translation());
    }
}
void Player::fixed_update(const mve::Window& window, const WorldData& data)
{
    mve::Vector3 dir(0.0f);
    if (window.is_key_down(mve::Key::w)) {
        dir.y += 1.0f;
    }
    if (window.is_key_down(mve::Key::s)) {
        dir.y -= 1.0f;
    }
    if (window.is_key_down(mve::Key::a)) {
        dir.x -= 1.0f;
    }
    if (window.is_key_down(mve::Key::d)) {
        dir.x += 1.0f;
    }
    if (window.is_key_down(mve::Key::space)) {
        dir.z += 1.0f;
    }
    if (window.is_key_down(mve::Key::left_shift)) {
        dir.z -= 1.0f;
    }
    m_velocity -= (m_velocity * m_friction);
    m_prev_pos = m_body_transform.translation();

    if (window.is_key_down(mve::Key::left_control)) {
        if (dir != mve::Vector3(0.0f)) {
            m_velocity += dir.normalize() * m_acceleration * 20.0f;
        }
    }
    else {
        if (dir != mve::Vector3(0.0f)) {
            m_velocity += dir.normalize() * m_acceleration;
        }
        if (m_velocity.length() > m_max_speed) {
            m_velocity = m_velocity * m_max_speed;
        }
    }

    mve::Matrix4 new_transform = m_body_transform.translate_local(m_velocity);
    mve::Vector3 vel = new_transform.translation() - m_body_transform.translation();
    //    m_body_transform = m_body_transform.translate(vel);

    auto detect_collision = [](mve::Vector3 vel, const BoundingBox& box, const WorldData& data) {
        BoundingBox broadphase_box = swept_broadphase_box(vel, box);
        SweptBoundingBoxCollision min_collision { .time = 1.0f };
        mve::Vector3i min_neighbor = mve::Vector3i(broadphase_box.min.round());
        mve::Vector3i max_neighbor = mve::Vector3i(broadphase_box.max.round()) + mve::Vector3i(1, 1, 1);
        for_3d(min_neighbor, max_neighbor, [&](mve::Vector3i neighbor) {
            mve::Vector3i block_pos = neighbor;
            std::optional<uint8_t> block = data.block_at(block_pos);
            BoundingBox block_bb { { mve::Vector3(block_pos) - mve::Vector3(0.5f) },
                                   { mve::Vector3(block_pos) + mve::Vector3(0.5f) } };
            if (block.has_value() && block.value() != 0 && collides(block_bb, broadphase_box)) {
                SweptBoundingBoxCollision collision = swept_bounding_box(vel, box, block_bb);
                if (collision.time < min_collision.time) {
                    min_collision = collision;
                }
            }
        });
        return min_collision;
    };

    mve::Vector3 cam_pos = m_body_transform.translation();
    BoundingBox cam_bb { { mve::Vector3(cam_pos) - mve::Vector3(0.25f) },
                         { mve::Vector3(cam_pos) + mve::Vector3(0.25f) } };

    SweptBoundingBoxCollision collision = detect_collision(vel, cam_bb, data);

    const float collision_padding = 0.001f;

    m_body_transform = m_body_transform.translate(vel * (collision.time - collision_padding));

    float remaining_time = 1.0f - collision.time;
    mve::Vector3 slide_vel = vel * (mve::Vector3(1.0f, 1.0f, 1.0f) - collision.normal.abs()) * remaining_time;

    cam_pos = m_body_transform.translation();
    cam_bb = { { mve::Vector3(cam_pos) - mve::Vector3(0.25f) }, { mve::Vector3(cam_pos) + mve::Vector3(0.25f) } };

    collision = detect_collision(slide_vel, cam_bb, data);

    m_body_transform = m_body_transform.translate(slide_vel * (collision.time - collision_padding));

    remaining_time = 1.0f - collision.time;
    slide_vel = slide_vel * (mve::Vector3(1.0f, 1.0f, 1.0f) - collision.normal.abs()) * remaining_time;

    cam_pos = m_body_transform.translation();
    cam_bb = { { mve::Vector3(cam_pos) - mve::Vector3(0.25f) }, { mve::Vector3(cam_pos) + mve::Vector3(0.25f) } };

    collision = detect_collision(slide_vel, cam_bb, data);

    m_body_transform = m_body_transform.translate(slide_vel * (collision.time - collision_padding));
}