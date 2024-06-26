#include "player.hpp"

#include <cereal/archives/portable_binary.hpp>

#include "world_data.hpp"
#include <mve/math/math.hpp>

Player::Player()
    : m_body_transform(mve::Matrix4f::identity().translate({ 0, 0, 2 }))
    , m_head_transform(mve::Matrix4f::identity())
    , m_prev_pos(mve::Vector3f(0, 0, 0))
    , m_friction(0.3f)
    , m_acceleration(0.035f)
    , m_max_speed(0.72f)
    , m_last_jump_time(std::chrono::steady_clock::now())
    , m_last_space_time(std::chrono::steady_clock::now())
    , m_is_flying(false)
    , m_save_loop(1.0f)
    , m_save(1024 * 1024, "player")
{
    if (std::optional<std::string> player_data = m_save.at<std::string>("pos"); player_data.has_value()) {
        std::stringstream data_stream(*player_data);
        cereal::PortableBinaryInputArchive archive_in(data_stream);
        archive_in(*this);
        m_body_transform = m_body_transform.translate({ 0, 0, 1 });
    }
}

void Player::update(const mve::Window& window, const bool capture_input)
{
    const mve::Vector2f mouse_delta = capture_input ? window.mouse_delta() : mve::Vector2f::zero();
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
        m_head_transform = mve::Matrix4f::from_basis_translation(
            mve::Matrix3f::from_euler({ mve::radians(0.1f), 0, 0 }), m_head_transform.translation());
    }
    if (head_euler.x > mve::radians(179.9f)) {
        m_head_transform = mve::Matrix4f::from_basis_translation(
            mve::Matrix3f::from_euler({ mve::radians(179.9f), 0, 0 }), m_head_transform.translation());
    }
    if (capture_input && window.is_key_pressed(mve::Key::space)) {
        if (const auto now = std::chrono::steady_clock::now();
            std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_space_time).count() <= 250) {
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
    m_save_loop.update(1, [this] { save_pos(); });
}
void Player::fixed_update(const mve::Window& window, const WorldData& data, const bool capture_input)
{
    m_prev_pos = m_body_transform.translation();
    const bool on_ground = is_on_ground(data);
    mve::Vector3f dir;
    if (capture_input) {
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
    }
    dir = dir.rotate(m_body_transform.basis().transpose());
    if (!m_is_flying) {
        if (on_ground) {
            m_velocity -= m_velocity * m_friction * 0.9f;
        }
        else {
            m_velocity.x -= m_velocity.x * m_friction * 0.1f;
            m_velocity.y -= m_velocity.y * m_friction * 0.1f;
        }
        m_velocity.z -= 0.014f;
    }
    else {
        m_velocity -= m_velocity * m_friction * 0.35f;
    }

    if (!m_is_flying && on_ground) {
        if (capture_input && window.is_key_down(mve::Key::space)
            && std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - m_last_jump_time)
                    .count()
                >= 200) {
            m_velocity.z = 0.21f;
            m_last_jump_time = std::chrono::steady_clock::now();
        }
    }
    if (!m_is_flying) {
        if (on_ground && dir != mve::Vector3f::zero()) {
            m_velocity += dir.normalized() * m_acceleration
                * mve::clamp(m_max_speed - mve::Vector2(m_velocity.x, m_velocity.y).length(), 0.0f, 1.0f);
        }
        else if (!on_ground && dir != mve::Vector3f::zero()) {
            m_velocity
                += dir.normalized() * m_acceleration * 0.1f * mve::clamp(m_max_speed - m_velocity.length(), 0.0f, 1.0f);
        }
    }
    else {
        if (capture_input && window.is_key_down(mve::Key::left_control)) {
            m_velocity
                += dir.normalized() * m_acceleration * mve::clamp(30.0f - m_velocity.length(), 0.0f, 1.0f) * 10.0f;
        }
        else {
            m_velocity
                += dir.normalized() * m_acceleration * mve::clamp(m_max_speed - m_velocity.length(), 0.0f, 1.0f) * 1.5f;
        }
    }

    m_velocity = move_and_slide(bounding_box(), m_body_transform, m_velocity, data);

    if (m_is_flying && is_on_ground(data)) {
        m_is_flying = false;
    }
}

mve::Vector3f Player::move_and_slide(
    BoundingBox box, mve::Matrix4f& transform, const mve::Vector3f velocity, const WorldData& data)
{
    auto detect_collision = [](const mve::Vector3f vel, const BoundingBox& bbox, const WorldData& world_data) {
        const BoundingBox broadphase_box = swept_broadphase_box(vel, bbox);
        SweptBoundingBoxCollision min_collision { .time = 1.0f, .normal = mve::Vector3f::zero() };
        const auto min_neighbor = mve::Vector3i(broadphase_box.min.rounded());
        const mve::Vector3i max_neighbor = mve::Vector3i(broadphase_box.max.rounded()) + mve::Vector3i(1, 1, 2);
        for_3d(min_neighbor, max_neighbor, [&](const mve::Vector3i neighbor) {
            const mve::Vector3i block_pos = neighbor - mve::Vector3i(0, 0, 1);
            const std::optional<uint8_t> block = world_data.block_at(block_pos);
            if (const BoundingBox block_bb { { mve::Vector3f(block_pos) - mve::Vector3f::all(0.5f) },
                                             { mve::Vector3f(block_pos) + mve::Vector3f::all(0.5f) } };
                block.has_value() && block.value() != 0 && collides(block_bb, broadphase_box)) {
                if (const SweptBoundingBoxCollision collision = swept_bounding_box(vel, bbox, block_bb);
                    collision.time < min_collision.time) {
                    min_collision = collision;
                }
            }
        });
        return min_collision;
    };

    SweptBoundingBoxCollision collision = detect_collision(velocity, box, data);

    constexpr float collision_padding = 0.001f;

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
    const mve::Vector3i block_pos = block_position();
    bool is_on_ground = false;
    for_2d({ -1, -1 }, { 2, 2 }, [&](const mve::Vector2i neighbor) {
        const BoundingBox block_bb {
            { mve::Vector3f(block_pos + mve::Vector3i(neighbor.x, neighbor.y, -1)) - mve::Vector3f::all(0.5f) },
            { mve::Vector3f(block_pos + mve::Vector3i(neighbor.x, neighbor.y, -1)) + mve::Vector3f::all(0.5f) }
        };
        if (const std::optional<uint8_t> block = data.block_at(block_pos + mve::Vector3i(neighbor.x, neighbor.y, -1));
            block.has_value() && block.value() != 0 && collides(bb, block_bb)) {
            is_on_ground = true;
        }
    });
    return is_on_ground;
}
Player::~Player()
{
    save_pos();
}
void Player::save_pos()
{
    m_save.insert<std::string, Player>("pos", *this);
}
