#pragma once

#include "math.h"

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
