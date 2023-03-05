#include "player.hpp"

#include <filesystem>
#include <limits>

#include <cereal/archives/portable_binary.hpp>
#include <leveldb/db.h>

#include "mve/common.hpp"
#include "mve/math/math.hpp"
#include "world_data.hpp"

Player::Player()
    : m_body_transform(mve::Matrix4::identity().translate({ 0, 0, 2 }))
    , m_head_transform(mve::Matrix4::identity())
    , m_prev_pos(mve::Vector3(0, 0, 0))
    , m_friction(0.3f)
    , m_acceleration(0.035f)
    , m_max_speed(0.72f)
    , m_last_jump_time(std::chrono::steady_clock::now())
    , m_last_space_time(std::chrono::steady_clock::now())
    , m_is_flying(false)
    , m_save_loop(1.0f)
{
    // TODO: Abstract leveldb management in its own class
    MVE_ASSERT(std::filesystem::exists("save"), "[Player] save dir does not exit")
    leveldb::Options db_options;
    db_options.create_if_missing = true;
    db_options.compression = leveldb::kNoCompression;
    db_options.max_file_size = 1024 * 1024; // 1 MB
    leveldb::Status db_status = leveldb::DB::Open(db_options, "save/player", &m_save_db);
    MVE_ASSERT(db_status.ok(), "[Player] Leveldb open not ok")

    std::string pos_data;
    db_status = m_save_db->Get(leveldb::ReadOptions(), "pos", &pos_data);
    if (!db_status.IsNotFound()) {
        std::stringstream data_stream(pos_data);
        cereal::PortableBinaryInputArchive archive_in(data_stream);
        archive_in(*this);
        m_body_transform = m_body_transform.translate({ 0, 0, 0.5 });
    }
    MVE_ASSERT(db_status.ok(), "[Player] Failed to get pos data")
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
    if (window.is_key_pressed(mve::Key::space)) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_space_time).count() <= 250) {
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
    m_save_loop.update(1, [this]() { save_pos(); });
}
void Player::fixed_update(const mve::Window& window, const WorldData& data)
{
    m_prev_pos = m_body_transform.translation();
    bool on_ground = is_on_ground(data);
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
    if (m_is_flying && window.is_key_down(mve::Key::space)) {
        dir.z += 1.0f;
    }
    if (m_is_flying && window.is_key_down(mve::Key::left_shift)) {
        dir.z -= 1.0f;
    }
    dir = dir.rotate(m_body_transform.basis().transpose());
    if (!m_is_flying) {
        if (on_ground) {
            m_velocity -= (m_velocity * m_friction) * 0.9f;
        }
        else {
            m_velocity.x -= (m_velocity.x * m_friction * 0.1f);
            m_velocity.y -= (m_velocity.y * m_friction * 0.1f);
        }
        m_velocity.z -= 0.014f;
    }
    else {
        m_velocity -= (m_velocity * m_friction) * 0.35f;
    }

    if (!m_is_flying && on_ground) {
        if (window.is_key_down(mve::Key::space)
            && std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - m_last_jump_time)
                    .count()
                >= 200) {
            m_velocity.z = 0.21f;
            m_last_jump_time = std::chrono::steady_clock::now();
        }
    }
    if (!m_is_flying) {
        if (on_ground && dir != mve::Vector3(0.0f)) {
            m_velocity += dir.normalize() * m_acceleration
                * mve::clamp((m_max_speed - mve::Vector2(m_velocity.x, m_velocity.y).length()), 0.0f, 1.0f);
        }
        else if (!on_ground && dir != mve::Vector3(0.0f)) {
            m_velocity += dir.normalize() * m_acceleration * 0.1f
                * mve::clamp((m_max_speed - m_velocity.length()), 0.0f, 1.0f);
        }
    }
    else {
        if (window.is_key_down(mve::Key::left_control)) {
            m_velocity
                += dir.normalize() * m_acceleration * mve::clamp((30.0f - m_velocity.length()), 0.0f, 1.0f) * 10.0f;
        }
        else {
            m_velocity += dir.normalize() * m_acceleration * mve::clamp((m_max_speed - m_velocity.length()), 0.0f, 1.0f)
                * 1.5f;
        }
    }

    m_velocity = move_and_slide(bounding_box(), m_body_transform, m_velocity, data);

    if (m_is_flying && is_on_ground(data)) {
        m_is_flying = false;
    }
}

mve::Vector3 Player::move_and_slide(
    BoundingBox box, mve::Matrix4& transform, mve::Vector3 velocity, const WorldData& data)
{
    auto detect_collision = [](mve::Vector3 vel, const BoundingBox& box, const WorldData& data) {
        BoundingBox broadphase_box = swept_broadphase_box(vel, box);
        SweptBoundingBoxCollision min_collision { .time = 1.0f, .normal = mve::Vector3(0.0f) };
        mve::Vector3i min_neighbor = mve::Vector3i(broadphase_box.min.round());
        mve::Vector3i max_neighbor = mve::Vector3i(broadphase_box.max.round()) + mve::Vector3i(1, 1, 2);
        for_3d(min_neighbor, max_neighbor, [&](mve::Vector3i neighbor) {
            mve::Vector3i block_pos = neighbor - mve::Vector3i(0, 0, 1);
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

    SweptBoundingBoxCollision collision = detect_collision(velocity, box, data);

    const float collision_padding = 0.001f;

    transform = transform.translate(velocity * (collision.time - collision_padding));
    box.min += velocity * (collision.time - collision_padding);
    box.max += velocity * (collision.time - collision_padding);

    float remaining_time = 1.0f - collision.time;
    if (remaining_time == 0.0f) {
        return velocity;
    }
    mve::Vector3 slide_vel = velocity * (mve::Vector3(1.0f, 1.0f, 1.0f) - collision.normal.abs()) * remaining_time;

    collision = detect_collision(slide_vel, box, data);

    transform = transform.translate(slide_vel * (collision.time - collision_padding));
    box.min += velocity * (collision.time - collision_padding);
    box.max += velocity * (collision.time - collision_padding);

    remaining_time = 1.0f - collision.time;
    if (remaining_time == 0.0f) {
        return slide_vel;
    }
    slide_vel = slide_vel * (mve::Vector3(1.0f, 1.0f, 1.0f) - collision.normal.abs()) * remaining_time;

    collision = detect_collision(slide_vel, box, data);

    transform = transform.translate(slide_vel * (collision.time - collision_padding));

    return slide_vel;
}

bool Player::is_on_ground(const WorldData& data) const
{
    BoundingBox bb = bounding_box();
    bb.min += mve::Vector3(0.001f, 0.001f, -0.001f);
    bb.max += mve::Vector3(-0.001f, -0.001f, -0.001f);
    mve::Vector3i block_pos = (m_body_transform.translation() - mve::Vector3(0.0f, 0.0f, 1.5f)).round();
    bool is_on_ground = false;
    for_2d({ -1, -1 }, { 2, 2 }, [&](mve::Vector2i neighbor) {
        BoundingBox block_bb {
            { mve::Vector3(block_pos + mve::Vector3i(neighbor.x, neighbor.y, -1)) - mve::Vector3(0.5f) },
            { mve::Vector3(block_pos + mve::Vector3i(neighbor.x, neighbor.y, -1)) + mve::Vector3(0.5f) }
        };
        std::optional<uint8_t> block = data.block_at(block_pos + mve::Vector3i(neighbor.x, neighbor.y, -1));
        if (block.has_value() && block.value() != 0 && collides(bb, block_bb)) {
            is_on_ground = true;
        }
    });
    return is_on_ground;
}
Player::~Player()
{
    save_pos();
    delete m_save_db;
}
void Player::save_pos()
{
    std::stringstream data;
    {
        cereal::PortableBinaryOutputArchive archive_out(data);
        archive_out(*this);
    }
    m_save_db->Put(leveldb::WriteOptions(), "pos", data.str());
}
