#include "camera.hpp"

#include "math/matrix3.hpp"

Camera::Camera()
    : m_body_transform(mve::Matrix4::identity())
    , m_head_transform(mve::Matrix4::identity())
    , m_prev_pos({ 0, 0, 0 })
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
}
void Camera::fixed_update(const mve::Window& window)
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
    if (dir != mve::Vector3(0.0f)) {
        m_velocity += dir.normalize() * m_acceleration;
    }
    if (m_velocity.length() > m_max_speed) {
        m_velocity = m_velocity.normalize() * m_max_speed;
    }
    m_body_transform = m_body_transform.translate_local(m_velocity);
}
mve::Matrix4 Camera::view_matrix(float interpolation_weight) const
{
    mve::Matrix4 transform = m_head_transform * m_body_transform;
    mve::Matrix3 basis = transform.basis();
    mve::Matrix4 interpolated_transform = mve::Matrix4::from_basis_translation(
        basis, m_prev_pos.linear_interpolate(transform.translation(), interpolation_weight));
    mve::Matrix4 view = interpolated_transform.inverse().transpose();
    return view;
}
mve::Vector3 Camera::position() const
{
    return m_body_transform.translation();
}
