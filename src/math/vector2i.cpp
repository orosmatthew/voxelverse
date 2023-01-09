#include "vector2i.hpp"

#include "functions.hpp"
#include "vector2.hpp"

namespace mve {

Vector2i::Vector2i()
    : x(0.0f)
    , y(0.0f)
{
}
Vector2i::Vector2i(const Vector2& vector)
    : x(static_cast<int>(vector.x))
    , y(static_cast<int>(vector.y))
{
}
Vector2i::Vector2i(int x, int y)
    : x(x)
    , y(y)
{
}
Vector2i::Vector2i(int val)
    : x(val)
    , y(val)
{
}
Vector2i Vector2i::abs() const
{
    return mve::abs(*this);
}
float Vector2i::aspect_ratio() const
{
    return mve::aspect_ratio(*this);
}
Vector2i Vector2i::clamp(const Vector2i& min, const Vector2i& max) const
{
    return mve::clamp(*this, min, max);
}
float Vector2i::length() const
{
    return mve::length(*this);
}
float Vector2i::length_squared() const
{
    return mve::length_squared(*this);
}
Vector2iAxis Vector2i::max_axis() const
{
    return mve::max_axis(*this);
}
Vector2iAxis Vector2i::min_axis() const
{
    return mve::min_axis(*this);
}
Vector2i Vector2i::zero()
{
    return Vector2i(0.0f, 0.0f);
}
Vector2i Vector2i::one()
{
    return Vector2i(1.0f, 1.0f);
}
bool Vector2i::operator!=(const Vector2i& other) const
{
    return x != other.x || y != other.y;
}
Vector2i Vector2i::operator%(const Vector2i& other) const
{
    return Vector2i(x % other.x, y % other.y);
}
Vector2i Vector2i::operator%(int val) const
{
    return Vector2i(x % val, y % val);
}
Vector2i Vector2i::operator*(const Vector2i& other) const
{
    return Vector2i(x * other.x, y * other.y);
}
Vector2i Vector2i::operator*(float val) const
{
    return Vector2i(x * val, y * val);
}
Vector2i Vector2i::operator*(int val) const
{
    return Vector2i(x * val, y * val);
}
Vector2i Vector2i::operator+(const Vector2i& other) const
{
    return Vector2i(x + other.x, y + other.y);
}
Vector2i Vector2i::operator-(const Vector2i& other) const
{
    return Vector2i(x - other.x, y - other.y);
}
Vector2i Vector2i::operator/(const Vector2i& other) const
{
    return Vector2i(x / other.x, y / other.y);
}
Vector2i Vector2i::operator/(float val) const
{
    return Vector2i(static_cast<float>(x) / val, static_cast<float>(y) / val);
}
Vector2i Vector2i::operator/(int val) const
{
    return Vector2i(x / val, y / val);
}
bool Vector2i::operator<(const Vector2i& other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return false;
}
bool Vector2i::operator<=(const Vector2i& other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return true;
}
bool Vector2i::operator==(const Vector2i& other) const
{
    return x == other.x && y == other.y;
}
bool Vector2i::operator>(const Vector2i& other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    return false;
}
bool Vector2i::operator>=(const Vector2i& other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    return true;
}
int& Vector2i::operator[](int index)
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
int& Vector2i::operator[](Vector2iAxis axis)
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
Vector2i Vector2i::operator+() const
{
    return Vector2i(x, y);
}
Vector2i Vector2i::operator-() const
{
    return Vector2i(-x, -y);
}
Vector2i& Vector2i::operator%=(const Vector2i& other)
{
    x %= other.x;
    y %= other.y;

    return *this;
}
Vector2i& Vector2i::operator%=(int val)
{
    x %= val;
    y %= val;

    return *this;
}
Vector2i& Vector2i::operator*=(const Vector2i& other)
{
    x *= other.x;
    y *= other.y;

    return *this;
}
Vector2i& Vector2i::operator*=(float val)
{
    x *= val;
    y *= val;

    return *this;
}
Vector2i& Vector2i::operator*=(int val)
{
    x *= val;
    y *= val;

    return *this;
}
Vector2i& Vector2i::operator+=(const Vector2i& other)
{
    x += other.x;
    y += other.y;

    return *this;
}
Vector2i& Vector2i::operator-=(const Vector2i& other)
{
    x -= other.x;
    y -= other.y;

    return *this;
}
Vector2i& Vector2i::operator/=(const Vector2i& other)
{
    x /= other.x;
    y /= other.y;

    return *this;
}
Vector2i& Vector2i::operator/=(float val)
{
    x /= val;
    y /= val;

    return *this;
}
Vector2i& Vector2i::operator/=(int val)
{
    x /= val;
    y /= val;

    return *this;
}
Vector2i::operator bool() const
{
    return x != 0 || y != 0;
}
Vector2i abs(const Vector2i& vector)
{
    return Vector2i(abs(vector.x), abs(vector.y));
}
float aspect_ratio(const Vector2i& vector)
{
    return static_cast<float>(vector.x) / static_cast<float>(vector.y);
}
Vector2i clamp(const Vector2i& vector, const Vector2i& min, const Vector2i& max)
{
    return Vector2i(clamp(vector.x, min.x, max.x), clamp(vector.y, min.y, max.y));
}
float length(const Vector2i& vector)
{
    return sqrt(squared(vector.x) + squared(vector.y));
}
float length_squared(const Vector2i& vector)
{
    return squared(vector.x) + squared(vector.y);
}
Vector2iAxis max_axis(const Vector2i& vector)
{
    if (vector.x > vector.y) {
        return Vector2iAxis::x;
    }
    if (vector.y > vector.x) {
        return Vector2iAxis::y;
    }
    return Vector2iAxis::x;
}
Vector2iAxis min_axis(const Vector2i& vector)
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