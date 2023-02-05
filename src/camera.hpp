#pragma once

#include "math/matrix4.hpp"
#include "math/vector3.hpp"
#include "window.hpp"

class Camera {
public:
    Camera();

    void update(const mve::Window& window);
    void fixed_update(const mve::Window& window);

    mve::Vector3 position() const;

    mve::Matrix4 view_matrix(float interpolation_weight) const;

    mve::Vector3 direction() const;

private:
    mve::Matrix4 m_body_transform;
    mve::Matrix4 m_head_transform;
    mve::Vector3 m_prev_pos;
    float m_friction;
    float m_acceleration;
    float m_max_speed;
    mve::Vector3 m_velocity {};
};