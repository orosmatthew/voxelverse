#pragma once

namespace mve {

inline Vector4i abs(const Vector4i vector)
{
    return { abs(vector.x), abs(vector.y), abs(vector.z), abs(vector.w) };
}
inline Vector4i clamp(const Vector4i vector, const Vector4i min, const Vector4i max)
{
    return { clamp(vector.x, min.x, max.x),
             clamp(vector.y, min.y, max.y),
             clamp(vector.z, min.z, max.z),
             clamp(vector.w, min.w, max.w) };
}
inline float length(const Vector4i vector)
{
    return sqrt(
        sqrd(static_cast<float>(vector.x)) + sqrd(static_cast<float>(vector.y)) + sqrd(static_cast<float>(vector.z))
        + sqrd(static_cast<float>(vector.w)));
}
inline float length_sqrd(const Vector4i vector)
{
    return sqrd(static_cast<float>(vector.x)) + sqrd(static_cast<float>(vector.y)) + sqrd(static_cast<float>(vector.z))
        + sqrd(static_cast<float>(vector.w));
}
inline Vector4iAxis max_axis(const Vector4i vector)
{
    int max_val = vector.x;
    Vector4iAxis max_axis = Vector4iAxis::x;

    if (vector.y > max_val) {
        max_val = vector.y;
        max_axis = Vector4iAxis::y;
    }

    if (vector.z > max_val) {
        max_val = vector.z;
        max_axis = Vector4iAxis::z;
    }

    if (vector.w > max_val) {
        max_axis = Vector4iAxis::w;
    }

    return max_axis;
}
inline Vector4iAxis min_axis(const Vector4i vector)
{
    int min_val = vector.x;
    Vector4iAxis min_axis = Vector4iAxis::x;

    if (vector.y < min_val) {
        min_val = vector.y;
        min_axis = Vector4iAxis::y;
    }

    if (vector.z < min_val) {
        min_val = vector.z;
        min_axis = Vector4iAxis::z;
    }

    if (vector.w < min_val) {
        min_axis = Vector4iAxis::w;
    }

    return min_axis;
}
inline Vector4i::Vector4i()
    : x(0)
    , y(0)
    , z(0)
    , w(0)
{
}
inline Vector4i::Vector4i(const int x, const int y, const int z, const int w)
    : x(x)
    , y(y)
    , z(z)
    , w(w)
{
}
inline Vector4i::Vector4i(const int val)
    : x(val)
    , y(val)
    , z(val)
    , w(val)
{
}
inline Vector4i Vector4i::zero()
{
    return { 0, 0, 0, 0 };
}
inline Vector4i Vector4i::one()
{
    return { 1, 1, 1, 1 };
}
inline Vector4i Vector4i::abs() const
{
    return mve::abs(*this);
}
inline Vector4i Vector4i::clamp(const Vector4i min, const Vector4i max) const
{
    return mve::clamp(*this, min, max);
}
inline float Vector4i::length() const
{
    return mve::length(*this);
}
inline float Vector4i::length_sqrd() const
{
    return mve::length_sqrd(*this);
}
inline Vector4iAxis Vector4i::max_axis() const
{
    return mve::max_axis(*this);
}
inline Vector4iAxis Vector4i::min_axis() const
{
    return mve::min_axis(*this);
}
inline bool Vector4i::operator!=(const Vector4i other) const
{
    return x != other.x || y != other.y || z != other.z || w != other.w;
}
inline Vector4i Vector4i::operator%(const Vector4i other) const
{
    return { x % other.x, y & other.y, z % other.z, w % other.w };
}
inline Vector4i& Vector4i::operator%=(const Vector4i other)
{
    x %= other.x;
    y %= other.y;
    z %= other.z;
    w %= other.w;

    return *this;
}
inline Vector4i Vector4i::operator%(const int val) const
{
    return { x % val, y % val, z % val, w % val };
}
inline Vector4i& Vector4i::operator%=(const int val)
{
    x %= val;
    y %= val;
    z %= val;
    w %= val;

    return *this;
}
inline Vector4i Vector4i::operator*(const Vector4i other) const
{
    return { x * other.x, y * other.y, z * other.z, w * other.w };
}
inline Vector4i& Vector4i::operator*=(const Vector4i other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;

    return *this;
}
inline Vector4i Vector4i::operator*(const float val) const
{
    return {
        x * static_cast<int>(val), y * static_cast<int>(val), z * static_cast<int>(val), w * static_cast<int>(val)
    };
}
inline Vector4i& Vector4i::operator*=(const float val)
{
    x *= static_cast<int>(val);
    y *= static_cast<int>(val);
    z *= static_cast<int>(val);
    w *= static_cast<int>(val);

    return *this;
}
inline Vector4i Vector4i::operator+(const Vector4i other) const
{
    return { x + other.x, y + other.y, z + other.z, w + other.w };
}
inline Vector4i& Vector4i::operator+=(const Vector4i other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;

    return *this;
}
inline Vector4i Vector4i::operator-(const Vector4i other) const
{
    return { x - other.x, y - other.y, z - other.z, w - other.w };
}
inline Vector4i& Vector4i::operator-=(const Vector4i other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;

    return *this;
}
inline Vector4i Vector4i::operator/(const Vector4i other) const
{
    return { x / other.x, y / other.y, z / other.z, w / other.w };
}
inline Vector4i& Vector4i::operator/=(const Vector4i other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;

    return *this;
}
inline Vector4i Vector4i::operator/(const float val) const
{
    return {
        x / static_cast<int>(val), y / static_cast<int>(val), z / static_cast<int>(val), w / static_cast<int>(val)
    };
}
inline Vector4i& Vector4i::operator/=(const float val)
{
    x /= static_cast<int>(val);
    y /= static_cast<int>(val);
    z /= static_cast<int>(val);
    w /= static_cast<int>(val);

    return *this;
}
inline Vector4i Vector4i::operator/(const int val) const
{
    return { x / val, y / val, z / val, w / val };
}
inline Vector4i& Vector4i::operator/=(const int val)
{
    x /= val;
    y /= val;
    z /= val;
    w /= val;

    return *this;
}
inline bool Vector4i::operator<(const Vector4i other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    if (z != other.z) {
        return z < other.z;
    }
    if (w != other.w) {
        return w < other.w;
    }
    return false;
}
inline bool Vector4i::operator<=(const Vector4i other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    if (z != other.z) {
        return z < other.z;
    }
    if (w != other.w) {
        return w < other.w;
    }
    return true;
}
inline bool Vector4i::operator==(const Vector4i other) const
{
    return x == other.x && y == other.y && z == other.z && w == other.w;
}
inline bool Vector4i::operator>(const Vector4i other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    if (z != other.z) {
        return z > other.z;
    }
    if (w != other.w) {
        return w > other.w;
    }
    return false;
}
inline bool Vector4i::operator>=(const Vector4i other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    if (z != other.z) {
        return z > other.z;
    }
    if (w != other.w) {
        return w > other.w;
    }
    return true;
}
inline int& Vector4i::operator[](const int index)
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    case 3:
        return w;
    default:
        return x;
    }
}
inline int& Vector4i::operator[](const Vector4iAxis axis)
{
    switch (axis) {
    case Vector4iAxis::x:
        return x;
    case Vector4iAxis::y:
        return y;
    case Vector4iAxis::z:
        return z;
    case Vector4iAxis::w:
        return w;
    default:
        return x;
    }
}
inline Vector4i Vector4i::operator+() const
{
    return { x, y, z, w };
}
inline Vector4i Vector4i::operator-() const
{
    return { -x, -y, -z, -w };
}
inline Vector4i::operator bool() const
{
    return x != 0 && y != 0 && z != 0 && w != 0;
}
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
