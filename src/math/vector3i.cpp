#include "vector3i.hpp"

#include "functions.hpp"
#include "vector3.hpp"

namespace mve {

Vector3i abs(Vector3i vector)
{
    return Vector3i(abs(vector.x), abs(vector.y), abs(vector.z));
}
Vector3i clamp(Vector3i vector, Vector3i min, Vector3i max)
{
    return Vector3i(clamp(vector.x, min.x, max.x), clamp(vector.y, min.y, max.y), clamp(vector.z, min.z, max.z));
}
float length(Vector3i vector)
{
    return sqrt(squared(vector.x) + squared(vector.y) + squared(vector.z));
}
float length_squared(Vector3i vector)
{
    return squared(vector.x) + squared(vector.y) + squared(vector.z);
}
Vector3iAxis max_axis(Vector3i vector)
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
Vector3iAxis min_axis(Vector3i vector)
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

Vector3i::Vector3i()
    : x(0.0f)
    , y(0.0f)
    , z(0.0f)
{
}
Vector3i::Vector3i(Vector3 vector)
    : x(static_cast<int>(vector.x))
    , y(static_cast<int>(vector.y))
    , z(static_cast<int>(vector.z))
{
}
Vector3i::Vector3i(int x, int y, int z)
    : x(x)
    , y(y)
    , z(z)
{
}
Vector3i::Vector3i(int val)
    : x(val)
    , y(val)
    , z(val)
{
}
Vector3i Vector3i::zero()
{
    return Vector3i(0.0f, 0.0f, 0.0f);
}
Vector3i Vector3i::one()
{
    return Vector3i(1.0f, 1.0f, 1.0f);
}
Vector3i Vector3i::abs() const
{
    return mve::abs(*this);
}
Vector3i Vector3i::clamp(Vector3i min, Vector3i max) const
{
    return mve::clamp(*this, min, max);
}
float Vector3i::length() const
{
    return mve::length(*this);
}
float Vector3i::length_squared() const
{
    return mve::length_squared(*this);
}
Vector3iAxis Vector3i::max_axis() const
{
    return mve::max_axis(*this);
}
Vector3iAxis Vector3i::min_axis() const
{
    return mve::min_axis(*this);
}
bool Vector3i::operator!=(Vector3i other) const
{
    return x != other.x || y != other.y || z != other.z;
}
Vector3i Vector3i::operator%(Vector3i other) const
{
    return Vector3i(x % other.x, y % other.y, z % other.z);
}
Vector3i& Vector3i::operator%=(Vector3i other)
{
    x %= other.x;
    y %= other.y;
    z %= other.z;

    return *this;
}
Vector3i Vector3i::operator%(int val) const
{
    return Vector3i(x % val, y % val, z % val);
}
Vector3i& Vector3i::operator%=(int val)
{
    x %= val;
    y %= val;
    z %= val;

    return *this;
}
Vector3i Vector3i::operator*(Vector3i other) const
{
    return Vector3i(x * other.x, y * other.y, z * other.z);
}
Vector3i& Vector3i::operator*=(Vector3i other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;

    return *this;
}
Vector3i Vector3i::operator*(float val) const
{
    return Vector3i(x * val, y * val, z * val);
}
Vector3i& Vector3i::operator*=(float val)
{
    x *= val;
    y *= val;
    z *= val;

    return *this;
}
Vector3i Vector3i::operator+(Vector3i other) const
{
    return Vector3i(x + other.x, y + other.y, z + other.z);
}
Vector3i& Vector3i::operator+=(Vector3i other)
{
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}
Vector3i Vector3i::operator-(Vector3i other) const
{
    return Vector3i(x - other.x, y - other.y, z - other.z);
}
Vector3i& Vector3i::operator-=(Vector3i other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;
}
Vector3i Vector3i::operator/(Vector3i other) const
{
    return Vector3i(x / other.x, y / other.y, z / other.z);
}
Vector3i& Vector3i::operator/=(Vector3i other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;

    return *this;
}
Vector3i Vector3i::operator/(float val) const
{
    return Vector3i(x / val, y / val, z / val);
}
Vector3i& Vector3i::operator/=(float val)
{
    x /= val;
    y /= val;
    z /= val;

    return *this;
}
Vector3i Vector3i::operator/(int val) const
{
    return Vector3i(x / val, y / val, z / val);
}
Vector3i& Vector3i::operator/=(int val)
{
    x /= val;
    y /= val;
    z /= val;

    return *this;
}
bool Vector3i::operator<(Vector3i other) const
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
bool Vector3i::operator<=(Vector3i other) const
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
bool Vector3i::operator==(Vector3i other) const
{
    return x == other.x && y == other.y && z == other.z;
}
bool Vector3i::operator>(Vector3i other) const
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
bool Vector3i::operator>=(Vector3i other) const
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
int& Vector3i::operator[](int index)
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
int& Vector3i::operator[](Vector3iAxis axis)
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
Vector3i Vector3i::operator+() const
{
    return Vector3i(x, y, z);
}
Vector3i Vector3i::operator-() const
{
    return Vector3i(-x, -y, -z);
}
Vector3i::operator bool() const
{
    return x != 0 || y != 0 || z != 0;
}
}