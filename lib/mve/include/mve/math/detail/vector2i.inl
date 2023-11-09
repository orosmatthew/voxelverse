#pragma once

namespace mve {

inline Vector2i::Vector2i()
    : x(0.0f)
    , y(0.0f)
{
}
inline Vector2i::Vector2i(const Vector2& vector)
    : x(static_cast<int>(vector.x))
    , y(static_cast<int>(vector.y))
{
}
inline Vector2i::Vector2i(const int x, const int y)
    : x(x)
    , y(y)
{
}
inline Vector2i::Vector2i(const int val)
    : x(val)
    , y(val)
{
}
inline Vector2i Vector2i::abs() const
{
    return mve::abs(*this);
}
inline float Vector2i::aspect_ratio() const
{
    return mve::aspect_ratio(*this);
}
inline Vector2i Vector2i::clamp(const Vector2i& min, const Vector2i& max) const
{
    return mve::clamp(*this, min, max);
}
inline float Vector2i::length() const
{
    return mve::length(*this);
}
inline float Vector2i::length_sqrd() const
{
    return mve::length_sqrd(*this);
}
inline Vector2iAxis Vector2i::max_axis() const
{
    return mve::max_axis(*this);
}
inline Vector2iAxis Vector2i::min_axis() const
{
    return mve::min_axis(*this);
}
inline Vector2i Vector2i::zero()
{
    return { 0, 0 };
}
inline Vector2i Vector2i::one()
{
    return { 1, 1 };
}
inline bool Vector2i::operator!=(const Vector2i& other) const
{
    return x != other.x || y != other.y;
}
inline Vector2i Vector2i::operator%(const Vector2i& other) const
{
    return { x % other.x, y % other.y };
}
inline Vector2i Vector2i::operator%(const int val) const
{
    return { x % val, y % val };
}
inline Vector2i Vector2i::operator*(const Vector2i& other) const
{
    return { x * other.x, y * other.y };
}
inline Vector2i Vector2i::operator*(const float val) const
{
    return { x * static_cast<int>(val), y * static_cast<int>(val) };
}
inline Vector2i Vector2i::operator*(const int val) const
{
    return { x * val, y * val };
}
inline Vector2i Vector2i::operator+(const Vector2i& other) const
{
    return { x + other.x, y + other.y };
}
inline Vector2i Vector2i::operator-(const Vector2i& other) const
{
    return { x - other.x, y - other.y };
}
inline Vector2i Vector2i::operator/(const Vector2i& other) const
{
    return { x / other.x, y / other.y };
}
inline Vector2i Vector2i::operator/(const float val) const
{
    return { x / static_cast<int>(val), y / static_cast<int>(val) };
}
inline Vector2i Vector2i::operator/(const int val) const
{
    return { x / val, y / val };
}
inline bool Vector2i::operator<(const Vector2i& other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return false;
}
inline bool Vector2i::operator<=(const Vector2i& other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return true;
}
inline bool Vector2i::operator==(const Vector2i& other) const
{
    return x == other.x && y == other.y;
}
inline bool Vector2i::operator>(const Vector2i& other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    return false;
}
inline bool Vector2i::operator>=(const Vector2i& other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    return true;
}
inline int& Vector2i::operator[](const int index)
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    default:
        return x;
    }
}
inline int& Vector2i::operator[](const Vector2iAxis axis)
{
    switch (axis) {
    case Vector2iAxis::x:
        return x;
    case Vector2iAxis::y:
        return y;
    default:
        return x;
    }
}
inline Vector2i Vector2i::operator+() const
{
    return { x, y };
}
inline Vector2i Vector2i::operator-() const
{
    return { -x, -y };
}
inline Vector2i& Vector2i::operator%=(const Vector2i& other)
{
    x %= other.x;
    y %= other.y;

    return *this;
}
inline Vector2i& Vector2i::operator%=(const int val)
{
    x %= val;
    y %= val;

    return *this;
}
inline Vector2i& Vector2i::operator*=(const Vector2i& other)
{
    x *= other.x;
    y *= other.y;

    return *this;
}
inline Vector2i& Vector2i::operator*=(const float val)
{
    x *= static_cast<int>(val);
    y *= static_cast<int>(val);

    return *this;
}
inline Vector2i& Vector2i::operator*=(const int val)
{
    x *= val;
    y *= val;

    return *this;
}
inline Vector2i& Vector2i::operator+=(const Vector2i& other)
{
    x += other.x;
    y += other.y;

    return *this;
}
inline Vector2i& Vector2i::operator-=(const Vector2i& other)
{
    x -= other.x;
    y -= other.y;

    return *this;
}
inline Vector2i& Vector2i::operator/=(const Vector2i& other)
{
    x /= other.x;
    y /= other.y;

    return *this;
}
inline Vector2i& Vector2i::operator/=(const float val)
{
    x /= static_cast<int>(val);
    y /= static_cast<int>(val);

    return *this;
}
inline Vector2i& Vector2i::operator/=(const int val)
{
    x /= val;
    y /= val;

    return *this;
}
inline Vector2i::operator bool() const
{
    return x != 0 || y != 0;
}
inline Vector2i abs(const Vector2i& vector)
{
    return { abs(vector.x), abs(vector.y) };
}
inline float aspect_ratio(const Vector2i& vector)
{
    return static_cast<float>(vector.x) / static_cast<float>(vector.y);
}
inline Vector2i clamp(const Vector2i& vector, const Vector2i& min, const Vector2i& max)
{
    return { clamp(vector.x, min.x, max.x), clamp(vector.y, min.y, max.y) };
}
inline float length(const Vector2i& vector)
{
    return sqrt(sqrd(static_cast<float>(vector.x)) + sqrd(static_cast<float>(vector.y)));
}
inline float length_sqrd(const Vector2i& vector)
{
    return sqrd(static_cast<float>(vector.x)) + sqrd(static_cast<float>(vector.y));
}
inline Vector2iAxis max_axis(const Vector2i& vector)
{
    if (vector.x > vector.y) {
        return Vector2iAxis::x;
    }
    if (vector.y > vector.x) {
        return Vector2iAxis::y;
    }
    return Vector2iAxis::x;
}
inline Vector2iAxis min_axis(const Vector2i& vector)
{
    if (vector.x < vector.y) {
        return Vector2iAxis::x;
    }
    if (vector.y < vector.x) {
        return Vector2iAxis::y;
    }
    return Vector2iAxis::x;
}

}

template <>
struct std::hash<mve::Vector2i> {
    int operator()(const mve::Vector2i& vector) const noexcept
    {
        return (vector.x + vector.y + 1) * (vector.x + vector.y) / 2 + vector.y;
    }
};