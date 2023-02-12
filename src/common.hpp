#pragma once

#include "math.h"

struct Quad {
    mve::Vector3 top_left;
    mve::Vector3 top_right;
    mve::Vector3 bottom_right;
    mve::Vector3 bottom_left;
};

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
    for (mve::Vector3& vertex : vertices) {
        vertex = vertex.transform(matrix);
    }
}

inline void for_2d(mve::Vector2i from, mve::Vector2i to, std::function<void(mve::Vector2i)> func)
{
    for (int x = from.x; x < to.x; x++) {
        for (int y = from.y; y < to.y; y++) {
            std::invoke(func, mve::Vector2i(x, y));
        }
    }
}

inline void for_3d(mve::Vector3i from, mve::Vector3i to, std::function<void(mve::Vector3i)> func)
{
    for (int x = from.x; x < to.x; x++) {
        for (int y = from.y; y < to.y; y++) {
            for (int z = from.z; z < to.z; z++) {
                std::invoke(func, mve::Vector3i(x, y, z));
            }
        }
    }
}
