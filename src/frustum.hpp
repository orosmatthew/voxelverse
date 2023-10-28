#pragma once

#include "mve/math/math.hpp"
#include "player.hpp"

class Frustum {
public:
    inline void update_perspective(float angle, float ratio, float near_dist, float far_dist)
    {
        m_ratio = ratio;
        m_angle = angle / 2.0f;
        m_near_dist = near_dist;
        m_far_dist = far_dist;

        // compute width and height of the near and far quads
        m_tan_angle = mve::tan(m_angle);
        m_sphere_factor.y = 1.0f / mve::cos(m_angle);
        m_sphere_factor.x = 1.0f / mve::cos(mve::atan(m_tan_angle * ratio));

        m_near_size.y = m_near_dist * m_tan_angle;
        m_near_size.x = m_near_size.y * ratio;

        m_far_size.y = m_far_dist * m_tan_angle;
        m_far_size.x = m_far_size.y * ratio;
    }

    inline void update_camera(const Player& camera)
    {
        m_camera_pos = camera.position();
        // compute the Z axis of camera
        m_z_axis = (camera.position() - camera.target()).normalize();

        // X-axis of camera of given "up" vector and Z axis
        m_x_axis = camera.up().cross(m_z_axis).normalize();

        // the real "up" vector is the cross product of Z and X
        m_y_axis = m_z_axis.cross(m_x_axis);

        // compute the center of the near and far planes
        mve::Vector3 near_center = m_camera_pos - m_z_axis * m_near_dist;
        mve::Vector3 far_center = m_camera_pos - m_z_axis * m_far_dist;

        // compute the 8 corners of the frustum
        m_near_quad.top_left = near_center + m_y_axis * m_near_size.y - m_x_axis * m_near_size.x;
        m_near_quad.top_right = near_center + m_y_axis * m_near_size.y + m_x_axis * m_near_size.x;
        m_near_quad.bottom_left = near_center - m_y_axis * m_near_size.y - m_x_axis * m_near_size.x;
        m_near_quad.bottom_right = near_center - m_y_axis * m_near_size.y + m_x_axis * m_near_size.x;

        m_far_quad.top_left = far_center + m_y_axis * m_far_size.y - m_x_axis * m_far_size.x;
        m_far_quad.bottom_right = far_center - m_y_axis * m_far_size.y + m_x_axis * m_far_size.x;
        m_far_quad.top_right = far_center + m_y_axis * m_far_size.y + m_x_axis * m_far_size.x;
        m_far_quad.bottom_left = far_center - m_y_axis * m_far_size.y - m_x_axis * m_far_size.x;
    }

    [[nodiscard]] inline bool contains_point(mve::Vector3 point) const
    {
        mve::Vector3 point_from_center = point - m_camera_pos;

        // Test z
        float point_z_val = point_from_center.dot(-m_z_axis);
        if (point_z_val > m_far_dist || point_z_val < m_near_dist)
            return false;

        // Test y
        float point_y_val = point_from_center.dot(m_y_axis);
        float y_bounds = point_z_val * m_tan_angle;
        if (point_y_val > y_bounds || point_y_val < -y_bounds)
            return false;

        // Test x
        float point_x_val = point_from_center.dot(m_x_axis);
        float x_bounds = y_bounds * m_ratio;
        if (point_x_val > x_bounds || point_x_val < -x_bounds)
            return false;

        return true;
    }

    [[nodiscard]] inline bool contains_sphere(mve::Vector3 position, float radius) const
    {
        float d1, d2;
        float az, ax, ay, zz1, zz2;
        bool result = true;

        mve::Vector3 pos_from_center = position - m_camera_pos;

        az = pos_from_center.dot({ -m_z_axis.x, -m_z_axis.y, -m_z_axis.z });
        if (az > m_far_dist + radius || az < m_near_dist - radius)
            return false;

        ax = pos_from_center.dot(m_x_axis);
        zz1 = az * m_tan_angle * m_ratio;
        d1 = m_sphere_factor.x * radius;
        if (ax > zz1 + d1 || ax < -zz1 - d1)
            return (false);

        ay = pos_from_center.dot(m_y_axis);
        zz2 = az * m_tan_angle;
        d2 = m_sphere_factor.y * radius;
        if (ay > zz2 + d2 || ay < -zz2 - d2)
            return (false);

        if (az > m_far_dist - radius || az < m_near_dist + radius)
            result = true;
        if (ay > zz2 - d2 || ay < -zz2 + d2)
            result = true;
        if (ax > zz1 - d1 || ax < -zz1 + d1)
            result = true;

        return result;
    }

private:
    struct Quad {
        mve::Vector3 top_left;
        mve::Vector3 top_right;
        mve::Vector3 bottom_right;
        mve::Vector3 bottom_left;
    };

    Quad m_near_quad;
    Quad m_far_quad;
    mve::Vector3 m_x_axis;
    mve::Vector3 m_y_axis;
    mve::Vector3 m_z_axis;
    mve::Vector3 m_camera_pos;
    float m_near_dist {};
    float m_far_dist {};
    float m_ratio {};
    float m_angle {};
    float m_tan_angle {};
    mve::Vector2 m_sphere_factor;
    mve::Vector2 m_near_size;
    mve::Vector2 m_far_size;
};