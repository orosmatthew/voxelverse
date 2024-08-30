#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "common.hpp"

#include <nnm/nnm.hpp>

namespace nnm {
template <class Archive>
void serialize(Archive& archive, Vector2i& v)
{
    archive(v.x, v.y);
}

template <class Archive, typename Real>
void serialize(Archive& archive, Vector2<Real>& v)
{
    archive(v.x, v.y);
}

template <class Archive>
void serialize(Archive& archive, Vector3i& v)
{
    archive(v.x, v.y, v.z);
}

template <class Archive, typename Real>
void serialize(Archive& archive, Vector4<Real>& v)
{
    archive(v.x, v.y, v.z, v.w);
}

template <class Archive, typename Real>
void serialize(Archive& archive, Matrix4<Real>& m)
{
    archive(m.columns[0], m.columns[1], m.columns[2], m.columns[3]);
}

template <class Archive, typename Real>
void serialize(Archive& archive, Transform3<Real>& t)
{
    archive(t.matrix);
}

template <class Archive, typename Real>
void serialize(Archive& archive, Vector3<Real>& v)
{
    archive(v.x, v.y, v.z);
}
}

inline std::filesystem::path res_path(const std::filesystem::path& path)
{
    return std::filesystem::path(RES_PATH) / path;
}

struct Quad {
    nnm::Vector3f top_left;
    nnm::Vector3f top_right;
    nnm::Vector3f bottom_right;
    nnm::Vector3f bottom_left;
};

struct BoundingBox {
    nnm::Vector3f min;
    nnm::Vector3f max;
};

struct Rect3 {
    nnm::Vector3f pos;
    nnm::Vector3f size;
};

inline std::vector<std::pair<nnm::Vector3f, nnm::Vector3f>> rect3_to_edges(const Rect3& rect)
{
    std::vector<std::pair<nnm::Vector3f, nnm::Vector3f>> edges;
    edges.reserve(12);

    // Extract position and size of rectangle
    const nnm::Vector3f& p = rect.pos;
    const nnm::Vector3f& s = rect.size;

    // Define corners of rectangle
    const std::array corners
        = { nnm::Vector3f(p.x, p.y, p.z),
            nnm::Vector3f(p.x, p.y, p.z + s.z),
            nnm::Vector3f(p.x, p.y + s.y, p.z),
            nnm::Vector3f(p.x, p.y + s.y, p.z + s.z),
            nnm::Vector3f(p.x + s.x, p.y, p.z),
            nnm::Vector3f(p.x + s.x, p.y, p.z + s.z),
            nnm::Vector3f(p.x + s.x, p.y + s.y, p.z),
            nnm::Vector3f(p.x + s.x, p.y + s.y, p.z + s.z) };

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
    const nnm::Vector3 bottom_left_back(box.min.x, box.min.y, box.min.z);
    float width = box.max.x - box.min.x;
    float height = box.max.y - box.min.y;
    float depth = box.max.z - box.min.z;
    return { bottom_left_back, { width, height, depth } };
}

inline BoundingBox rect3_to_bounding_box(const Rect3& rect)
{
    const nnm::Vector3 min(rect.pos.x, rect.pos.y, rect.pos.z);
    const nnm::Vector3 max(min.x + rect.size.x, min.y + rect.size.y, min.z + rect.size.z);
    return { min, max };
}

inline BoundingBox swept_broadphase_box(const nnm::Vector3f vel, const BoundingBox& box)
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
    nnm::Vector3f normal;
};

inline SweptBoundingBoxCollision swept_bounding_box(nnm::Vector3f vel, const BoundingBox& b1, const BoundingBox& b2)
{
    nnm::Vector3f inv_entry;
    nnm::Vector3f inv_exit;

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

    nnm::Vector3f entry;
    nnm::Vector3f exit;

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

    const float entry_time = nnm::max(entry.x, nnm::max(entry.y, entry.z));
    const float exit_time = nnm::min(exit.x, nnm::min(exit.y, exit.z));

    // If no collision
    SweptBoundingBoxCollision collision;
    if (entry_time > exit_time || entry.x < 0.0f && entry.y < 0.0f && entry.z < 0.0f || entry.x > 1.0f || entry.y > 1.0f
        || entry.z > 1.0f) {
        collision.normal = nnm::Vector3f::zero();
        collision.time = 1.0f;
        return collision;
    }
    // If collision
    collision.normal = nnm::Vector3f::zero();
    const int max_axis = entry.max_index();
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

inline Quad transform(const Quad& quad, const nnm::Matrix4f& matrix)
{
    return {
        quad.top_left.transform(nnm::Transform3f(matrix)),
        quad.top_right.transform(nnm::Transform3f(matrix)),
        quad.bottom_right.transform(nnm::Transform3f(matrix)),
        quad.bottom_left.transform(nnm::Transform3f(matrix)),
    };
}

inline void transform_vertices(std::vector<nnm::Vector3f>& vertices, const nnm::Matrix4f& matrix)
{
    std::ranges::transform(vertices, vertices.begin(), [&](const nnm::Vector3f& vertex) {
        return vertex.transform(nnm::Transform3f(matrix));
    });
}

template <typename Callable>
concept CallableWithVector2i = requires(Callable callable, nnm::Vector2i vector) { callable(vector); };

template <CallableWithVector2i Callable>
void for_2d(const nnm::Vector2i from, const nnm::Vector2i to, Callable callable)
{
    for (int x = from.x; x < to.x; x++) {
        for (int y = from.y; y < to.y; y++) {
            std::invoke(callable, nnm::Vector2i(x, y));
        }
    }
}

template <typename Callable>
concept CallableWithVector3i = requires(Callable callable, nnm::Vector3i vector) { callable(vector); };

template <CallableWithVector3i Callable>
void for_3d(const nnm::Vector3i from, const nnm::Vector3i to, Callable callable)
{
    for (int x = from.x; x < to.x; ++x) {
        for (int y = from.y; y < to.y; ++y) {
            for (int z = from.z; z < to.z; ++z) {
                std::invoke(callable, nnm::Vector3i(x, y, z));
            }
        }
    }
}

struct QuadUVs {
    nnm::Vector2f top_left;
    nnm::Vector2f top_right;
    nnm::Vector2f bottom_right;
    nnm::Vector2f bottom_left;
};

inline QuadUVs uvs_from_atlas(const nnm::Vector2i atlas_size, const nnm::Vector2i pos)
{
    const auto atlas_unit
        = nnm::Vector2(1.0f / static_cast<float>(atlas_size.x), 1.0f / static_cast<float>(atlas_size.y));

    QuadUVs uvs;
    uvs.top_left = nnm::Vector2(static_cast<float>(pos.x) * atlas_unit.x, static_cast<float>(pos.y) * atlas_unit.y);
    uvs.top_right = uvs.top_left + nnm::Vector2(atlas_unit.x, 0.0f);
    uvs.bottom_right = uvs.top_right + nnm::Vector2(0.0f, atlas_unit.y);
    uvs.bottom_left = uvs.bottom_right + nnm::Vector2(-atlas_unit.x, 0.0f);

    return uvs;
}

inline nnm::Vector2i block_uv(const uint8_t block_type, const Direction face)
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

inline nnm::Vector3i chunk_pos_from_block_pos(const nnm::Vector3i block_pos)
{
    return { static_cast<int>(nnm::floor(static_cast<float>(block_pos.x) / 16.0f)),
             static_cast<int>(nnm::floor(static_cast<float>(block_pos.y) / 16.0f)),
             static_cast<int>(nnm::floor(static_cast<float>(block_pos.z) / 16.0f)) };
}

inline nnm::Vector3i block_world_to_local(const nnm::Vector3i world_block_pos)
{
    nnm::Vector3i mod = world_block_pos % 16;
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

inline nnm::Vector3i block_local_to_world(const nnm::Vector3i chunk_pos, const nnm::Vector3i local_block_pos)
{
    return { chunk_pos.x * 16 + local_block_pos.x,
             chunk_pos.y * 16 + local_block_pos.y,
             chunk_pos.z * 16 + local_block_pos.z };
}

inline nnm::Vector2i chunk_col_from_block_col(const nnm::Vector2i block_col)
{
    return { static_cast<int>(nnm::floor(static_cast<float>(block_col.x) / 16.0f)),
             static_cast<int>(nnm::floor(static_cast<float>(block_col.y) / 16.0f)) };
}

inline int chunk_height_from_block_height(const int block_height)
{
    return static_cast<int>(nnm::floor(static_cast<float>(block_height) / 16.0f));
}

inline nnm::Vector2i block_local_to_world_col(const nnm::Vector2i chunk_pos, const nnm::Vector2i local_block_pos)
{
    return { chunk_pos.x * 16 + local_block_pos.x, chunk_pos.y * 16 + local_block_pos.y };
}

inline int block_height_world_to_local(const int world_block_height)
{
    int mod = world_block_height % 16;
    if (mod < 0) {
        mod = 16 + mod;
    }
    return mod;
}

inline bool is_block_pos_local(const nnm::Vector3i block_pos)
{
    return block_pos.x >= 0 && block_pos.x < 16 && block_pos.y >= 0 && block_pos.y < 16 && block_pos.z >= 0
        && block_pos.z < 16;
}

inline bool is_block_pos_local_col(const nnm::Vector2i block_pos)
{
    return block_pos.x >= 0 && block_pos.x < 16 && block_pos.y >= 0 && block_pos.y < 16;
}

inline bool is_block_height_world_valid(const int height)
{
    return height >= -160 && height < 160;
}

template <typename T, typename Pred>
typename std::vector<T>::iterator insert_sorted(std::vector<T>& vec, T const& item, Pred pred)
{
    return vec.insert(std::upper_bound(vec.begin(), vec.end(), item, pred), item);
}