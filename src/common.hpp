#pragma once

#include "mve/math/math.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <utility>
#include <vector>

namespace mve {
template <class Archive>
void serialize(Archive& archive, Vector2i& v)
{
    archive(v.x, v.y);
}

template <class Archive>
void serialize(Archive& archive, Vector3i& v)
{
    archive(v.x, v.y, v.z);
}

template <class Archive>
void serialize(Archive& archive, Matrix4& m)
{
    archive(m.col0, m.col1, m.col2, m.col3);
}

template <class Archive>
void serialize(Archive& archive, Vector4& v)
{
    archive(v.x, v.y, v.z, v.w);
}

template <class Archive>
void serialize(Archive& archive, Vector3& v)
{
    archive(v.x, v.y, v.z);
}
}

inline std::filesystem::path res_path(const std::filesystem::path& path)
{
    return std::filesystem::path(RES_PATH) / path;
}

struct Quad {
    mve::Vector3 top_left;
    mve::Vector3 top_right;
    mve::Vector3 bottom_right;
    mve::Vector3 bottom_left;
};

struct BoundingBox {
    mve::Vector3 min;
    mve::Vector3 max;
};

struct Rect3 {
    mve::Vector3 pos;
    mve::Vector3 size;
};

inline std::vector<std::pair<mve::Vector3, mve::Vector3>> rect3_to_edges(const Rect3& rect)
{
    std::vector<std::pair<mve::Vector3, mve::Vector3>> edges;
    edges.reserve(12);

    // Extract position and size of rectangle
    const mve::Vector3& p = rect.pos;
    const mve::Vector3& s = rect.size;

    // Define corners of rectangle
    const std::array corners
        = { mve::Vector3(p.x, p.y, p.z),
            mve::Vector3(p.x, p.y, p.z + s.z),
            mve::Vector3(p.x, p.y + s.y, p.z),
            mve::Vector3(p.x, p.y + s.y, p.z + s.z),
            mve::Vector3(p.x + s.x, p.y, p.z),
            mve::Vector3(p.x + s.x, p.y, p.z + s.z),
            mve::Vector3(p.x + s.x, p.y + s.y, p.z),
            mve::Vector3(p.x + s.x, p.y + s.y, p.z + s.z) };

    // Define edges of rectangle
    edges.emplace_back(corners[0], corners[1]);
    edges.emplace_back(corners[0], corners[2]);
    edges.emplace_back(corners[0], corners[4]);
    edges.emplace_back(corners[1], corners[3]);
    edges.emplace_back(corners[1], corners[5]);
    edges.emplace_back(corners[2], corners[3]);
    edges.emplace_back(corners[2], corners[6]);
    edges.emplace_back(corners[3], corners[7]);
    edges.emplace_back(corners[4], corners[5]);
    edges.emplace_back(corners[4], corners[6]);
    edges.emplace_back(corners[5], corners[7]);
    edges.emplace_back(corners[6], corners[7]);

    return edges;
}

inline Rect3 bounding_box_to_rect3(const BoundingBox& box)
{
    const mve::Vector3 bottom_left_back(box.min.x, box.min.y, box.min.z);
    float width = box.max.x - box.min.x;
    float height = box.max.y - box.min.y;
    float depth = box.max.z - box.min.z;
    return { bottom_left_back, { width, height, depth } };
}

inline BoundingBox rect3_to_bounding_box(const Rect3& rect)
{
    const mve::Vector3 min(rect.pos.x, rect.pos.y, rect.pos.z);
    const mve::Vector3 max(min.x + rect.size.x, min.y + rect.size.y, min.z + rect.size.z);
    return { min, max };
}

inline BoundingBox swept_broadphase_box(const mve::Vector3 vel, const BoundingBox& box)
{
    BoundingBox broadphase_box;
    broadphase_box.min.x = vel.x > 0.0f ? box.min.x : box.min.x + vel.x;
    broadphase_box.min.y = vel.y > 0.0f ? box.min.y : box.min.y + vel.y;
    broadphase_box.min.z = vel.z > 0.0f ? box.min.z : box.min.z + vel.z;
    broadphase_box.max.x = vel.x > 0.0f ? vel.x + box.max.x : box.max.x - vel.x;
    broadphase_box.max.y = vel.y > 0.0f ? vel.y + box.max.y : box.max.y - vel.y;
    broadphase_box.max.z = vel.z > 0.0f ? vel.z + box.max.z : box.max.z - vel.z;
    return broadphase_box;
}

struct SweptBoundingBoxCollision {
    float time {};
    mve::Vector3 normal;
};

inline SweptBoundingBoxCollision swept_bounding_box(mve::Vector3 vel, const BoundingBox& b1, const BoundingBox& b2)
{
    mve::Vector3 inv_entry;
    mve::Vector3 inv_exit;

    auto [pos1, size1] = bounding_box_to_rect3(b1);
    auto [pos2, size2] = bounding_box_to_rect3(b2);

    for (int i = 0; i < 3; i++) {
        if (vel[i] > 0.0f) {
            inv_entry[i] = pos2[i] - (pos1[i] + size1[i]);
            inv_exit[i] = pos2[i] + size2[i] - pos1[i];
        }
        else {
            inv_entry[i] = pos2[i] + size2[i] - pos1[i];
            inv_exit[i] = pos2[i] - (pos1[i] + size1[i]);
        }
    }

    mve::Vector3 entry;
    mve::Vector3 exit;

    for (int i = 0; i < 3; i++) {
        if (vel[i] == 0.0f) {
            entry[i] = -std::numeric_limits<float>::infinity();
            exit[i] = std::numeric_limits<float>::infinity();
        }
        else {
            entry[i] = inv_entry[i] / vel[i];
            exit[i] = inv_exit[i] / vel[i];
        }
    }

    const float entry_time = mve::max(entry.x, mve::max(entry.y, entry.z));
    const float exit_time = mve::min(exit.x, mve::min(exit.y, exit.z));

    // If no collision
    SweptBoundingBoxCollision collision;
    if (entry_time > exit_time || entry.x < 0.0f && entry.y < 0.0f && entry.z < 0.0f || entry.x > 1.0f || entry.y > 1.0f
        || entry.z > 1.0f) {
        collision.normal = mve::Vector3(0.0f);
        collision.time = 1.0f;
        return collision;
    }
    // If collision
    collision.normal = mve::Vector3(0.0f);
    const mve::Vector3Axis max_axis = entry.max_axis();
    collision.normal[max_axis] = inv_entry[max_axis] < 0.0f ? 1.0f : -1.0f;
    collision.time = entry_time;
    return collision;
}

enum class Direction { front = 0, back, left, right, top, bottom };

inline bool collides(const BoundingBox& a, const BoundingBox& b)
{
    bool collision = true;

    if (a.max.x >= b.min.x && a.min.x <= b.max.x) {
        if (a.max.y < b.min.y || a.min.y > b.max.y) {
            collision = false;
        }
        if (a.max.z < b.min.z || a.min.z > b.max.z) {
            collision = false;
        }
    }
    else {
        collision = false;
    }

    return collision;
}

inline Quad transform(const Quad& quad, const mve::Matrix4& matrix)
{
    return {
        quad.top_left.transform(matrix),
        quad.top_right.transform(matrix),
        quad.bottom_right.transform(matrix),
        quad.bottom_left.transform(matrix),
    };
}

inline void transform_vertices(std::vector<mve::Vector3>& vertices, const mve::Matrix4& matrix)
{
    std::ranges::transform(std::as_const(vertices), vertices.begin(), [&](const mve::Vector3& vertex) {
        return vertex.transform(matrix);
    });
}

inline void for_2d(const mve::Vector2i from, const mve::Vector2i to, std::function<void(mve::Vector2i)> func)
{
    for (int x = from.x; x < to.x; x++) {
        for (int y = from.y; y < to.y; y++) {
            std::invoke(func, mve::Vector2i(x, y));
        }
    }
}

inline void for_3d(const mve::Vector3i from, const mve::Vector3i to, std::function<void(mve::Vector3i)> func)
{
    for (int x = from.x; x < to.x; x++) {
        for (int y = from.y; y < to.y; y++) {
            for (int z = from.z; z < to.z; z++) {
                std::invoke(func, mve::Vector3i(x, y, z));
            }
        }
    }
}

struct QuadUVs {
    mve::Vector2 top_left;
    mve::Vector2 top_right;
    mve::Vector2 bottom_right;
    mve::Vector2 bottom_left;
};

inline QuadUVs uvs_from_atlas(const mve::Vector2i atlas_size, const mve::Vector2i pos)
{
    const mve::Vector2 atlas_unit
        = mve::Vector2(1.0f / static_cast<float>(atlas_size.x), 1.0f / static_cast<float>(atlas_size.y));

    QuadUVs uvs;
    uvs.top_left = mve::Vector2(static_cast<float>(pos.x) * atlas_unit.x, static_cast<float>(pos.y) * atlas_unit.y);
    uvs.top_right = uvs.top_left + mve::Vector2(atlas_unit.x, 0.0f);
    uvs.bottom_right = uvs.top_right + mve::Vector2(0.0f, atlas_unit.y);
    uvs.bottom_left = uvs.bottom_right + mve::Vector2(-atlas_unit.x, 0.0f);

    return uvs;
}

inline mve::Vector2i block_uv(const uint8_t block_type, const Direction face)
{
    switch (block_type) {
    case 1:
        switch (face) {
        case Direction::top:
            return { 1, 0 };
        case Direction::bottom:
            return { 0, 1 };
        default:
            return { 0, 0 };
        }
    case 2:
        return { 1, 1 };
    case 3:
        return { 2, 0 };
    case 4:
        return { 0, 1 };
    case 5:
        switch (face) {
        case Direction::top:
        case Direction::bottom:
            return { 3, 1 };
        default:
            return { 2, 1 };
        }
    case 6:
        return { 0, 2 };
    case 7:
        return { 1, 2 };
    case 8:
        return { 2, 2 };
    case 9:
        return { 3, 2 };
    case 10:
        return { 0, 3 };
    default:
        return { 0, 0 };
    }
}

inline bool is_transparent(const uint8_t block_type)
{
    switch (block_type) {
    case 0:
    case 9:
        return true;
    default:
        return false;
    }
}

inline bool is_emissive(const uint8_t block_type)
{
    return block_type == 10;
}

inline mve::Vector3i chunk_pos_from_block_pos(const mve::Vector3i block_pos)
{
    return { static_cast<int>(mve::floor(static_cast<float>(block_pos.x) / 16.0f)),
             static_cast<int>(mve::floor(static_cast<float>(block_pos.y) / 16.0f)),
             static_cast<int>(mve::floor(static_cast<float>(block_pos.z) / 16.0f)) };
}

inline mve::Vector3i block_world_to_local(const mve::Vector3i world_block_pos)
{
    mve::Vector3i mod = world_block_pos % 16;
    if (mod.x < 0) {
        mod.x = 16 + mod.x;
    }
    if (mod.y < 0) {
        mod.y = 16 + mod.y;
    }
    if (mod.z < 0) {
        mod.z = 16 + mod.z;
    }
    return mod;
}

inline mve::Vector3i block_local_to_world(const mve::Vector3i chunk_pos, const mve::Vector3i local_block_pos)
{
    return { chunk_pos.x * 16 + local_block_pos.x,
             chunk_pos.y * 16 + local_block_pos.y,
             chunk_pos.z * 16 + local_block_pos.z };
}