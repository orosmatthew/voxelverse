#pragma once

#include <functional>

namespace mve {

// Keep
inline Vector4i::Vector4i(const Vector4& vector)
    : x(static_cast<int>(vector.x))
    , y(static_cast<int>(vector.y))
    , z(static_cast<int>(vector.z))
    , w(static_cast<int>(vector.w))
{
}
}

template <>
struct std::hash<mve::Vector4i> {
    int operator()(const mve::Vector4i& vector) const noexcept
    {
        const int cantor_z_w = (vector.z + vector.w + 1) * (vector.z + vector.w) / 2 + vector.w;
        const int cantor_y_z_w = (vector.y + cantor_z_w + 1) * (vector.y + cantor_z_w) / 2 + cantor_z_w;
        return (vector.x + cantor_y_z_w) * (vector.x + cantor_y_z_w) / 2 + cantor_y_z_w;
    }
};
