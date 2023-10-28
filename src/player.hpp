#pragma once

#include <chrono>

#include <cereal/cereal.hpp>

#include "common.hpp"
#include "mve/math/math.hpp"
#include "mve/window.hpp"
#include "save_file.hpp"
#include "util/fixed_loop.hpp"

class WorldData;

class Player {

    friend cereal::access;

public:
    Player();

    ~Player();

    void update(const mve::Window& window, bool capture_input);
    void fixed_update(const mve::Window& window, const WorldData& data, bool capture_input);

    [[nodiscard]] inline mve::Vector3 position() const
    {
        return m_body_transform.translation();
    }

    [[nodiscard]] inline mve::Vector3i block_position() const
    {
        return mve::Vector3i(m_body_transform.translate({ 0.0f, 0.0f, -1.62f }).translation().round());
    }

    [[nodiscard]] inline mve::Matrix4 view_matrix(float interpolation_weight) const
    {
        mve::Matrix4 transform = m_head_transform * m_body_transform;
        mve::Matrix3 basis = transform.basis();
        mve::Matrix4 interpolated_transform = mve::Matrix4::from_basis_translation(
            basis, m_prev_pos.linear_interpolate(transform.translation(), interpolation_weight));
        mve::Matrix4 view = interpolated_transform.inverse().transpose();
        return view;
    }

    [[nodiscard]] inline mve::Vector3 direction() const
    {
        mve::Matrix4 transform = m_head_transform * m_body_transform;
        mve::Matrix3 basis = transform.basis().transpose();
        mve::Vector3 direction { 0, 0, -1 };
        direction = direction.rotate(basis);
        return direction.normalize();
    }

    [[nodiscard]] inline mve::Vector3 target() const
    {
        return position() + direction();
    }

    [[nodiscard]] static inline mve::Vector3 up()
    {
        return { 0, 0, 1 };
    }

    [[nodiscard]] inline BoundingBox bounding_box() const
    {
        mve::Vector3 pos = m_body_transform.translation();
        return { { mve::Vector3(pos) - mve::Vector3(0.3f, 0.3f, 1.62f) },
                 { mve::Vector3(pos) + mve::Vector3(0.3f, 0.3f, 0.18f) } };
    }

    [[nodiscard]] inline mve::Vector3 velocity() const
    {
        return m_velocity;
    }

private:
    void save_pos();

    [[nodiscard]] bool is_on_ground(const WorldData& data) const;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(m_body_transform, m_head_transform, m_prev_pos, m_velocity, m_is_flying);
    }

    mve::Matrix4 m_body_transform;
    mve::Matrix4 m_head_transform;
    mve::Vector3 m_prev_pos;
    float m_friction;
    float m_acceleration;
    float m_max_speed;
    mve::Vector3 m_velocity;
    std::chrono::time_point<std::chrono::steady_clock> m_last_jump_time;
    std::chrono::time_point<std::chrono::steady_clock> m_last_space_time;
    bool m_is_flying;
    util::FixedLoop m_save_loop;
    SaveFile m_save;

    static mve::Vector3 move_and_slide(
        BoundingBox box, mve::Matrix4& transform, mve::Vector3 velocity, const WorldData& data);
};