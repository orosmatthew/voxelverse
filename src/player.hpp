#pragma once

#include "common.hpp"
#include "mve/math/math.hpp"
#include "mve/window.hpp"

class WorldData;

class Player {
public:
    Player();

    void update(const mve::Window& window);
    void fixed_update(const mve::Window& window, const WorldData& data);

    inline mve::Vector3 position() const
    {
        return m_body_transform.translation();
    }

    inline mve::Matrix4 view_matrix(float interpolation_weight) const
    {
        mve::Matrix4 transform = m_head_transform * m_body_transform;
        mve::Matrix3 basis = transform.basis();
        mve::Matrix4 interpolated_transform = mve::Matrix4::from_basis_translation(
            basis, m_prev_pos.linear_interpolate(transform.translation(), interpolation_weight));
        mve::Matrix4 view = interpolated_transform.inverse().transpose();
        return view;
    }

    inline mve::Vector3 direction() const
    {
        mve::Matrix4 transform = m_head_transform * m_body_transform;
        mve::Matrix3 basis = transform.basis().transpose();
        mve::Vector3 direction { 0, 0, -1 };
        direction = direction.rotate(basis);
        return direction.normalize();
    }

    inline mve::Vector3 target() const
    {
        return position() + direction();
    }

    inline mve::Vector3 up() const
    {
        return { 0, 0, 1 };
    }

private:
    inline BoundingBox bounding_box() const
    {
        mve::Vector3 pos = m_body_transform.translation();
        return { { mve::Vector3(pos) - mve::Vector3(0.3f, 0.3f, 1.62f) },
                 { mve::Vector3(pos) + mve::Vector3(0.3f, 0.3f, 0.18f) } };
    }

    bool is_on_ground(const WorldData& data) const;

    mve::Matrix4 m_body_transform;
    mve::Matrix4 m_head_transform;
    mve::Vector3 m_prev_pos;
    float m_friction;
    float m_acceleration;
    float m_max_speed;
    mve::Vector3 m_velocity;
    std::chrono::time_point<std::chrono::steady_clock> m_last_jump_time;

    static mve::Vector3 move_and_slide(
        BoundingBox box, mve::Matrix4& transform, mve::Vector3 velocity, const WorldData& data);
};