#include "vector4i.hpp"

#include "functions.hpp"

namespace mve {

Vector4i abs(Vector4i vector)
{
    return Vector4i(abs(vector.x), abs(vector.y), abs(vector.z), abs(vector.w));
}
Vector4i clamp(Vector4i vector, Vector4i min, Vector4i max)
{
    return Vector4i(
        clamp(vector.x, min.x, max.x),
        clamp(vector.y, min.y, max.y),
        clamp(vector.z, min.z, max.z),
        clamp(vector.w, min.w, max.w));
}
float length(Vector4i vector)
{
    return sqrt(squared(vector.x) + squared(vector.y) + squared(vector.z) + squared(vector.w));
}
float length_squared(Vector4i vector)
{
    return squared(vector.x) + squared(vector.y) + squared(vector.z) + squared(vector.w);
}
Vector4iAxis max_axis(Vector4i vector)
{
    float max_val = vector.x;
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
Vector4iAxis min_axis(Vector4i vector)
{
    float min_val = vector.x;
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
Vector4i::Vector4i()
    : x(0)
    , y(0)
    , z(0)
    , w(0)
{
}
Vector4i::Vector4i(int x, int y, int z, int w)
    : x(x)
    , y(y)
    , z(z)
    , w(w)
{
}
Vector4i::Vector4i(int val)
    : x(val)
    , y(val)
    , z(val)
    , w(val)
{
}
Vector4i Vector4i::zero()
{
    return Vector4i(0, 0, 0, 0);
}
Vector4i Vector4i::one()
{
    return Vector4i(1, 1, 1, 1);
}
Vector4i Vector4i::abs() const
{
    return mve::abs(*this);
}
Vector4i Vector4i::clamp(Vector4i min, Vector4i max) const
{
    return mve::clamp(*this, min, max);
}
float Vector4i::length() const
{
    return mve::length(*this);
}
float Vector4i::length_squared() const
{
    return mve::length_squared(*this);
}
Vector4iAxis Vector4i::max_axis() const
{
    return mve::max_axis(*this);
}
Vector4iAxis Vector4i::min_axis() const
{
    return mve::min_axis(*this);
}
bool Vector4i::operator!=(Vector4i other) const
{
    return x != other.x || y != other.y || z != other.z || w != other.w;
}
Vector4i Vector4i::operator%(Vector4i other) const
{
    return Vector4i(x % other.x, y & other.y, z % other.z, w % other.w);
}
Vector4i& Vector4i::operator%=(Vector4i other)
{
    x %= other.x;
    y %= other.y;
    z %= other.z;
    w %= other.w;

    return *this;
}
Vector4i Vector4i::operator%(int val) const
{
    return Vector4i(x % val, y % val, z % val, w % val);
}
Vector4i& Vector4i::operator%=(int val)
{
    x %= val;
    y %= val;
    z %= val;
    w %= val;

    return *this;
}
Vector4i Vector4i::operator*(Vector4i other) const
{
    return Vector4i(x * other.x, y * other.y, z * other.z, w * other.w);
}
Vector4i& Vector4i::operator*=(Vector4i other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;

    return *this;
}
Vector4i Vector4i::operator*(float val) const
{
    return Vector4i(x * val, y * val, z * val, w * val);
}
Vector4i& Vector4i::operator*=(float val)
{
    x *= val;
    y *= val;
    z *= val;
    w *= val;

    return *this;
}
Vector4i Vector4i::operator+(Vector4i other) const
{
    return Vector4i(x + other.x, y + other.y, z + other.z, w + other.w);
}
Vector4i& Vector4i::operator+=(Vector4i other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;

    return *this;
}
Vector4i Vector4i::operator-(Vector4i other) const
{
    return Vector4i(x - other.x, y - other.y, z - other.z, w - other.w);
}
Vector4i& Vector4i::operator-=(Vector4i other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;

    return *this;
}
Vector4i Vector4i::operator/(Vector4i other) const
{
    return Vector4i(x / other.x, y / other.y, z / other.z, w / other.w);
}
Vector4i& Vector4i::operator/=(Vector4i other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;

    return *this;
}
Vector4i Vector4i::operator/(float val) const
{
    return Vector4i(x / val, y / val, z / val, w / val);
}
Vector4i& Vector4i::operator/=(float val)
{
    x /= val;
    y /= val;
    z /= val;
    w /= val;

    return *this;
}
Vector4i Vector4i::operator/(int val) const
{
    return Vector4i(x / val, y / val, z / val, w / val);
}
Vector4i& Vector4i::operator/=(int val)
{
    x /= val;
    y /= val;
    z /= val;
    w /= val;

    return *this;
}
bool Vector4i::operator<(Vector4i other) const
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
bool Vector4i::operator<=(Vector4i other) const
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
bool Vector4i::operator==(Vector4i other) const
{
    return x == other.x && y == other.y && z == other.z && w == other.w;
}
bool Vector4i::operator>(Vector4i other) const
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
bool Vector4i::operator>=(Vector4i other) const
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
int& Vector4i::operator[](int index)
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
int& Vector4i::operator[](Vector4iAxis axis)
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
Vector4i Vector4i::operator+() const
{
    return Vector4i(x, y, z, w);
}
Vector4i Vector4i::operator-() const
{
    return Vector4i(-x, -y, -z, -w);
}
Vector4i::operator bool() const
{
    return x != 0 && y != 0 && z != 0 && w != 0;
}
}