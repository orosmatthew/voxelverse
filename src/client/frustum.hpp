#pragma once

#include "common.hpp"

#include <nnm/nnm.hpp>

#include "player.hpp"

class Frustum {
public:
    void update_perspective(const float angle, const float ratio, const float near_dist, const float far_dist)
    {
        m_ratio = ratio;
        m_angle = angle / 2.0f;
        m_near_dist = near_dist;
        m_far_dist = far_dist;

        // compute width and height of the near and far quads
        m_tan_angle = nnm::tan(m_angle);
        m_sphere_factor.y = 1.0f / nnm::cos(m_angle);
        m_sphere_factor.x = 1.0f / nnm::cos(nnm::atan(m_tan_angle * ratio));

        m_near_size.y = m_near_dist * m_tan_angle;
        m_near_size.x = m_near_size.y * ratio;

        m_far_size.y = m_far_dist * m_tan_angle;
        m_far_size.x = m_far_size.y * ratio;
    }

    void update_camera(const Player& camera)
    {
        m_camera_pos = camera.position();
        // compute the Z axis of camera
        m_z_axis = (camera.position() - camera.target()).normalize();

        // X-axis of camera of given "up" vector and Z axis
        m_x_axis = Player::up().cross(m_z_axis).normalize();

        // the real "up" vector is the cross product of Z and X
        m_y_axis = m_z_axis.cross(m_x_axis);

        // compute the center of the near and far planes
        const nnm::Vector3f near_center = m_camera_pos - m_z_axis * m_near_dist;
        const nnm::Vector3f far_center = m_camera_pos - m_z_axis * m_far_dist;

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

    [[nodiscard]] bool contains_point(const nnm::Vector3f point) const
    {
        const nnm::Vector3f point_from_center = point - m_camera_pos;

        // Test z
        const float point_z_val = point_from_center.dot(-m_z_axis);
        if (point_z_val > m_far_dist || point_z_val < m_near_dist)
            return false;

        // Test y
        const float point_y_val = point_from_center.dot(m_y_axis);
        const float y_bounds = point_z_val * m_tan_angle;
        if (point_y_val > y_bounds || point_y_val < -y_bounds)
            return false;

        // Test x
        const float point_x_val = point_from_center.dot(m_x_axis);
        if (const float x_bounds = y_bounds * m_ratio; point_x_val > x_bounds || point_x_val < -x_bounds)
            return false;

        return true;
    }

    [[nodiscard]] bool contains_sphere(const nnm::Vector3f position, const float radius) const
    {
        bool result = true;

        const nnm::Vector3f pos_from_center = position - m_camera_pos;

        const float az = pos_from_center.dot({ -m_z_axis.x, -m_z_axis.y, -m_z_axis.z });
        if (az > m_far_dist + radius || az < m_near_dist - radius)
            return false;

        const float ax = pos_from_center.dot(m_x_axis);
        const float zz1 = az * m_tan_angle * m_ratio;
        const float d1 = m_sphere_factor.x * radius;
        if (ax > zz1 + d1 || ax < -zz1 - d1)
            return false;

        const float ay = pos_from_center.dot(m_y_axis);
        const float zz2 = az * m_tan_angle;
        const float d2 = m_sphere_factor.y * radius;
        if (ay > zz2 + d2 || ay < -zz2 - d2)
            return false;

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
        nnm::Vector3f top_left;
        nnm::Vector3f top_right;
        nnm::Vector3f bottom_right;
        nnm::Vector3f bottom_left;
    };

    Quad m_near_quad;
    Quad m_far_quad;
    nnm::Vector3f m_x_axis;
    nnm::Vector3f m_y_axis;
    nnm::Vector3f m_z_axis;
    nnm::Vector3f m_camera_pos;
    float m_near_dist {};
    float m_far_dist {};
    float m_ratio {};
    float m_angle {};
    float m_tan_angle {};
    nnm::Vector2f m_sphere_factor;
    nnm::Vector2f m_near_size;
    nnm::Vector2f m_far_size;
};