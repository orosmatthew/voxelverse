#include "camera.hpp"

#include "common.hpp"
#include "logger.hpp"
#include "mve/math/math.hpp"
#include "world_data.hpp"

Camera::Camera()
    : m_body_transform(mve::Matrix4::identity())
    , m_head_transform(mve::Matrix4::identity())
    , m_prev_pos(mve::Vector3(0, 0, 0))
    , m_friction(0.1f)
    , m_acceleration(0.03f)
    , m_max_speed(0.3f)
{
}

void Camera::update(const mve::Window& window)
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
void Camera::fixed_update(const mve::Window& window, const WorldData& data)
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

    m_body_transform = m_body_transform.translate_local(m_velocity);

    handle_collision(data);
}

void Camera::handle_collision(const WorldData& data)
{
    for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](mve::Vector3 neighbor) {
        mve::Vector3 pos = m_body_transform.translation();
        mve::Vector3i block_pos = pos.floor();
        BoundingBox cam_bb { { mve::Vector3(pos) - mve::Vector3(0.5f, 0.5f, 0.5f) },
                             { mve::Vector3(pos) + mve::Vector3(0.5f, 0.5f, 0.5f) } };
        std::optional<uint8_t> block = data.block_at(block_pos);
        BoundingBox block_bb { { mve::Vector3(neighbor + block_pos) - mve::Vector3(0.5f, 0.5f, 0.5f) },
                               { mve::Vector3(neighbor + block_pos) + mve::Vector3(0.5f, 0.5f, 0.5f) } };
        if (block.has_value() && block.value() != 0 && collides(cam_bb, block_bb)) {
            mve::Vector3 diff = pos - block_pos;
            mve::Vector3Axis max_axis = diff.abs().max_axis();
            float axis_diff = mve::Vector3(block_pos)[max_axis] - pos[max_axis];
            mve::Vector3 offset = mve::Vector3(0.0f);
            offset[max_axis] = axis_diff;
            mve::Vector3 new_pos = m_body_transform.translation() - offset;

            m_body_transform = mve::Matrix4::from_basis_translation(m_body_transform.basis(), new_pos);
        }
    });
}
