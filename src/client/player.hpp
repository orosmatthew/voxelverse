#pragma once

#include <chrono>

#include <cereal/cereal.hpp>
#include <nnm/nnm.hpp>

#include <mve/window.hpp>

#include "../common/fixed_loop.hpp"
#include "common.hpp"
#include "save_file.hpp"

class WorldData;

class Player {

    friend cereal::access;

public:
    Player();

    ~Player();

    void update(const mve::Window& window, bool capture_input);
    void fixed_update(const mve::Window& window, const WorldData& data, bool capture_input);

    [[nodiscard]] nnm::Vector3f position() const
    {
        return m_pos;
    }

    [[nodiscard]] nnm::Vector3i block_position() const
    {
        return nnm::Vector3i(m_pos.translate({ 0.0f, 0.0f, -1.62f }).round());
    }

    [[nodiscard]] nnm::Matrix4f view_matrix(const float interpolation_weight) const
    {
        // const nnm::Basis3f basis = transform.basis();
        // const nnm::Transform3f interpolated_transform = nnm::Transform3f::from_basis_translation(
        //     basis, m_prev_pos.lerp(transform.translation(), interpolation_weight));
        const nnm::Matrix4f view = transform().unchecked_inverse().matrix;
        return view;
    }

    [[nodiscard]] nnm::Vector3f direction() const
    {
        const nnm::Basis3f basis = transform().basis();
        nnm::Vector3f direction { 0, 0, -1 };
        direction = direction.transform(basis);
        return direction.normalize();
    }

    [[nodiscard]] nnm::Vector3f target() const
    {
        return position() + direction();
    }

    [[nodiscard]] static nnm::Vector3f up()
    {
        return { 0, 1, 0 };
    }

    [[nodiscard]] BoundingBox bounding_box() const
    {
        return { { nnm::Vector3(m_pos) - nnm::Vector3(0.3f, 0.3f, 1.62f) },
                 { nnm::Vector3(m_pos) + nnm::Vector3(0.3f, 0.3f, 0.18f) } };
    }

    [[nodiscard]] nnm::Vector3f velocity() const
    {
        return m_velocity;
    }

    [[nodiscard]] nnm::Transform3f transform() const
    {
        return nnm::Transform3f::from_rotation_axis_angle(nnm::Vector3f::axis_y(), m_head_rotation.x)
            .translate(m_pos)
            .rotate_axis_angle_local(nnm::Vector3f::axis_x(), m_head_rotation.y);
    }

private:
    void save_pos();

    [[nodiscard]] bool is_on_ground(const WorldData& data) const;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(m_pos, m_head_rotation, m_prev_pos, m_velocity, m_is_flying);
    }

    nnm::Vector3f m_pos;
    nnm::Vector2f m_head_rotation;
    nnm::Vector3f m_prev_pos;
    float m_friction;
    float m_acceleration;
    float m_max_speed;
    nnm::Vector3f m_velocity;
    std::chrono::time_point<std::chrono::steady_clock> m_last_jump_time;
    std::chrono::time_point<std::chrono::steady_clock> m_last_space_time;
    bool m_is_flying;
    util::FixedLoop m_save_loop;
    SaveFile m_save;

    static nnm::Vector3f move_and_slide(
        BoundingBox box, nnm::Vector3f& pos, nnm::Vector3f velocity, const WorldData& data);
};