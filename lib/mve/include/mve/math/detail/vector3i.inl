#pragma once

namespace mve {

inline Vector3i abs(const Vector3i vector)
{
    return { abs(vector.x), abs(vector.y), abs(vector.z) };
}
inline Vector3i clamp(const Vector3i vector, const Vector3i min, const Vector3i max)
{
    return { clamp(vector.x, min.x, max.x), clamp(vector.y, min.y, max.y), clamp(vector.z, min.z, max.z) };
}
inline float length(const Vector3i vector)
{
    return sqrt(
        sqrd(static_cast<float>(vector.x)) + sqrd(static_cast<float>(vector.y)) + sqrd(static_cast<float>(vector.z)));
}
inline float length_sqrd(const Vector3i vector)
{
    return sqrd(static_cast<float>(vector.x)) + sqrd(static_cast<float>(vector.y)) + sqrd(static_cast<float>(vector.z));
}
inline Vector3iAxis max_axis(const Vector3i vector)
{
    int max_val = vector.x;
    Vector3iAxis max_axis = Vector3iAxis::x;
    if (vector.y > max_val) {
        max_val = vector.y;
        max_axis = Vector3iAxis::y;
    }
    if (vector.z > max_val) {
        max_axis = Vector3iAxis::z;
    }
    return max_axis;
}
inline Vector3iAxis min_axis(const Vector3i vector)
{
    int min_val = vector.x;
    Vector3iAxis min_axis = Vector3iAxis::x;
    if (vector.y < min_val) {
        min_val = vector.y;
        min_axis = Vector3iAxis::y;
    }
    if (vector.z < min_val) {
        min_axis = Vector3iAxis::z;
    }
    return min_axis;
}

inline Vector3i::Vector3i()
    : x(0.0f)
    , y(0.0f)
    , z(0.0f)
{
}
inline Vector3i::Vector3i(const Vector3 vector)
    : x(static_cast<int>(vector.x))
    , y(static_cast<int>(vector.y))
    , z(static_cast<int>(vector.z))
{
}
inline Vector3i::Vector3i(const int x, const int y, const int z)
    : x(x)
    , y(y)
    , z(z)
{
}
inline Vector3i::Vector3i(const int val)
    : x(val)
    , y(val)
    , z(val)
{
}
inline Vector3i Vector3i::zero()
{
    return { 0, 0, 0 };
}
inline Vector3i Vector3i::one()
{
    return { 1, 1, 1 };
}
inline Vector3i Vector3i::abs() const
{
    return mve::abs(*this);
}
inline Vector3i Vector3i::clamp(const Vector3i min, const Vector3i max) const
{
    return mve::clamp(*this, min, max);
}
inline float Vector3i::length() const
{
    return mve::length(*this);
}
inline float Vector3i::length_sqrd() const
{
    return mve::length_sqrd(*this);
}
inline Vector3iAxis Vector3i::max_axis() const
{
    return mve::max_axis(*this);
}
inline Vector3iAxis Vector3i::min_axis() const
{
    return mve::min_axis(*this);
}
inline bool Vector3i::operator!=(const Vector3i other) const
{
    return x != other.x || y != other.y || z != other.z;
}
inline Vector3i Vector3i::operator%(const Vector3i other) const
{
    return { x % other.x, y % other.y, z % other.z };
}
inline Vector3i& Vector3i::operator%=(const Vector3i other)
{
    x %= other.x;
    y %= other.y;
    z %= other.z;

    return *this;
}
inline Vector3i Vector3i::operator%(const int val) const
{
    return { x % val, y % val, z % val };
}
inline Vector3i& Vector3i::operator%=(const int val)
{
    x %= val;
    y %= val;
    z %= val;

    return *this;
}
inline Vector3i Vector3i::operator*(const Vector3i other) const
{
    return { x * other.x, y * other.y, z * other.z };
}
inline Vector3i& Vector3i::operator*=(const Vector3i other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;

    return *this;
}
inline Vector3i Vector3i::operator*(const float val) const
{
    return { x * static_cast<int>(val), y * static_cast<int>(val), z * static_cast<int>(val) };
}
inline Vector3i& Vector3i::operator*=(const float val)
{
    x *= static_cast<int>(val);
    y *= static_cast<int>(val);
    z *= static_cast<int>(val);

    return *this;
}
inline Vector3i Vector3i::operator+(const Vector3i& other) const
{
    return { x + other.x, y + other.y, z + other.z };
}
inline Vector3i& Vector3i::operator+=(const Vector3i& other)
{
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}
inline Vector3i Vector3i::operator-(const Vector3i other) const
{
    return { x - other.x, y - other.y, z - other.z };
}
inline Vector3i& Vector3i::operator-=(const Vector3i other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;
}
inline Vector3i Vector3i::operator/(const Vector3i other) const
{
    return { x / other.x, y / other.y, z / other.z };
}
inline Vector3i& Vector3i::operator/=(const Vector3i other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;

    return *this;
}
inline Vector3i Vector3i::operator/(const float val) const
{
    return { x / static_cast<int>(val), y / static_cast<int>(val), z / static_cast<int>(val) };
}
inline Vector3i& Vector3i::operator/=(const float val)
{
    x /= static_cast<int>(val);
    y /= static_cast<int>(val);
    z /= static_cast<int>(val);

    return *this;
}
inline Vector3i Vector3i::operator/(const int val) const
{
    return { x / val, y / val, z / val };
}
inline Vector3i& Vector3i::operator/=(const int val)
{
    x /= val;
    y /= val;
    z /= val;

    return *this;
}
inline bool Vector3i::operator<(const Vector3i other) const
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
    return false;
}
inline bool Vector3i::operator<=(const Vector3i other) const
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
    return true;
}
inline bool Vector3i::operator==(const Vector3i other) const
{
    return x == other.x && y == other.y && z == other.z;
}
inline bool Vector3i::operator>(const Vector3i other) const
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
    return false;
}
inline bool Vector3i::operator>=(const Vector3i other) const
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
    return true;
}
inline int& Vector3i::operator[](const int index)
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        return x;
    }
}
inline int& Vector3i::operator[](const Vector3iAxis axis)
{
    switch (axis) {
    case Vector3iAxis::x:
        return x;
    case Vector3iAxis::y:
        return y;
    case Vector3iAxis::z:
        return z;
    default:
        return x;
    }
}
inline Vector3i Vector3i::operator+() const
{
    return { x, y, z };
}
inline Vector3i Vector3i::operator-() const
{
    return { -x, -y, -z };
}

}

template <>
struct std::hash<mve::Vector3i> {
    int operator()(const mve::Vector3i& vector) const noexcept
    {
        const int cantor_y_z = (vector.y + vector.z + 1) * (vector.y + vector.z) / 2 + vector.z;
        return (vector.x + cantor_y_z) * (vector.x + cantor_y_z) / 2 + cantor_y_z;
    }
};