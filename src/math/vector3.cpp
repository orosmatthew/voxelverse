#include "vector3.hpp"

#include "functions.hpp"

namespace mve {

Vector3 abs(Vector3 vector)
{
    return Vector3(abs(vector.x), abs(vector.y), abs(vector.z));
}
Vector3 ceil(Vector3 vector)
{
    return Vector3(ceil(vector.x), ceil(vector.y), ceil(vector.z));
}
Vector3 clamp(Vector3 vector, Vector3 min, Vector3 max)
{
    return Vector3(clamp(vector.x, min.x, max.x), clamp(vector.y, min.y, max.y), clamp(vector.z, min.z, max.z));
}
Vector3 direction_to(Vector3 from, Vector3 to)
{
    return normalize(to - from);
}
float distance_squared_to(Vector3 from, Vector3 to)
{
    float diff_x = to.x - from.x;
    float diff_y = to.y - from.y;
    float diff_z = to.z - from.z;
    return squared(diff_x) + squared(diff_y) + squared(diff_z);
}
float distance_to(Vector3 from, Vector3 to)
{
    float diff_x = to.x - from.x;
    float diff_y = to.y - from.y;
    float diff_z = to.z - from.z;
    return sqrt(squared(diff_x) + squared(diff_y) + squared(diff_z));
}
Vector3 normalize(Vector3 vector)
{
    Vector3 result(0.0f);
    float length = mve::length(vector);

    if (length > 0) {
        float inverse_length = 1.0f / length;
        result = vector * inverse_length;
    }

    return result;
}
Vector3 floor(Vector3 vector)
{
    return Vector3(floor(vector.x), floor(vector.y), floor(vector.z));
}
float length(Vector3 vector)
{
    return sqrt(squared(vector.x) + squared(vector.y) + squared(vector.z));
}
float length_squared(Vector3 vector)
{
    return squared(vector.x) + squared(vector.y) + squared(vector.z);
}
Vector3 linear_interpolate(Vector3 from, Vector3 to, float weight)
{
    return Vector3(
        linear_interpolate(from.x, to.x, weight),
        linear_interpolate(from.y, to.y, weight),
        linear_interpolate(from.z, to.z, weight));
}
Vector3Axis max_axis(Vector3 vector)
{
    float max_val = vector.x;
    Vector3Axis max_axis = Vector3Axis::x;

    if (vector.y > max_val) {
        max_val = vector.y;
        max_axis = Vector3Axis::y;
    }

    if (vector.z > max_val) {
        max_val = vector.z;
        max_axis = Vector3Axis::z;
    }

    return max_axis;
}
Vector3Axis min_axis(Vector3 vector)
{
    float min_val = vector.x;
    Vector3Axis min_axis = Vector3Axis::x;

    if (vector.y < min_val) {
        min_val = vector.y;
        min_axis = Vector3Axis::y;
    }

    if (vector.z < min_val) {
        min_val = vector.z;
        min_axis = Vector3Axis::z;
    }

    return min_axis;
}
bool is_equal_approx(Vector3 a, Vector3 b)
{
    return is_equal_approx(a.x, b.x) && is_equal_approx(a.y, b.y) && is_equal_approx(a.z, b.z);
}
bool is_zero_approx(Vector3 vector)
{
    return is_zero_approx(vector.x) && is_zero_approx(vector.y) && is_zero_approx(vector.z);
}
Vector3 move_toward(Vector3 from, Vector3 to, float amount)
{
    if (from.distance_to(to) >= amount) {
        return to;
    }
    Vector3 result = from;
    result += from.direction_to(to) * amount;
    return result;
}
float dot(Vector3 a, Vector3 b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}
Vector3 cross(Vector3 a, Vector3 b)
{
    return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
Vector3 reflect(Vector3 vector, Vector3 normal)
{
    Vector3 result;

    float dot = (vector.x * normal.x + vector.y * normal.y + vector.z * normal.z);

    result.x = vector.x - (2.0f * normal.x) * dot;
    result.y = vector.y - (2.0f * normal.y) * dot;
    result.z = vector.z - (2.0f * normal.z) * dot;

    return result;
}
Vector3 inverse(Vector3 vector)
{
    return Vector3(1.0f / vector.x, 1.0f / vector.y, 1.0f / vector.z);
}
Vector3 clamp_length(Vector3 vector, float min, float max)
{
    float length_sqr = mve::length_squared(vector);
    if (length_sqr > 0.0f) {
        float length = sqrt(length_sqr);
        if (length < min) {
            return normalize(vector) * min;
        }
        else if (length > max) {
            return normalize(vector) * max;
        }
    }
    return vector;
}
Vector3 round(Vector3 vector)
{
    return Vector3(round(vector.x), round(vector.y), round(vector.z));
}
float angle(Vector3 a, Vector3 b)
{
    return atan2(mve::length(mve::cross(a, b)), mve::dot(a, b));
}
Vector3 rotate(Vector3 vector, Vector3 axis, float angle)
{
    axis = mve::normalize(axis);

    angle /= 2.0f;
    float a = sin(angle);
    float b = axis.x * a;
    float c = axis.y * a;
    float d = axis.z * a;
    a = cos(angle);
    Vector3 w(b, c, d);

    Vector3 wv = mve::cross(w, vector);

    Vector3 wwv = mve::cross(w, wv);

    wv *= 2.0f * a;

    wwv *= 2.0f;

    Vector3 result = vector;

    result += wv;

    result += wwv;

    return result;
}

Vector3::Vector3()
    : x(0.0f)
    , y(0.0f)
    , z(0.0f)
{
}
Vector3::Vector3(float val)
    : x(val)
    , y(val)
    , z(val)
{
}
Vector3::Vector3(float x, float y, float z)
    : x(x)
    , y(y)
    , z(z)
{
}
Vector3 Vector3::zero()
{
    return Vector3(0.0f, 0.0f, 0.0f);
}
Vector3 Vector3::one()
{
    return Vector3(1.0f, 1.0f, 1.0f);
}
Vector3 Vector3::abs() const
{
    return mve::abs(*this);
}
Vector3 Vector3::ceil() const
{
    return mve::ceil(*this);
}
Vector3 Vector3::clamp(Vector3 min, Vector3 max) const
{
    return mve::clamp(*this, min, max);
}
Vector3 Vector3::direction_to(Vector3 to) const
{
    return mve::direction_to(*this, to);
}
float Vector3::distance_squared_to(Vector3 to) const
{
    return mve::distance_squared_to(*this, to);
}
float Vector3::distance_to(Vector3 to) const
{
    return mve::distance_to(*this, to);
}
Vector3 Vector3::normalize() const
{
    return mve::normalize(*this);
}
Vector3 Vector3::floor() const
{
    return mve::floor(*this);
}
float Vector3::length() const
{
    return mve::length(*this);
}
float Vector3::length_squared() const
{
    return mve::length_squared(*this);
}
Vector3 Vector3::linear_interpolate(Vector3 to, float weight) const
{
    return mve::linear_interpolate(*this, to, weight);
}
Vector3Axis Vector3::max_axis() const
{
    return mve::max_axis(*this);
}
Vector3Axis Vector3::min_axis() const
{
    return mve::min_axis(*this);
}
bool Vector3::is_equal_approx(Vector3 vector) const
{
    return mve::is_equal_approx(*this, vector);
}
bool Vector3::is_zero_approx() const
{
    return mve::is_zero_approx(*this);
}
Vector3 Vector3::move_toward(Vector3 to, float amount) const
{
    return mve::move_toward(*this, to, amount);
}
float Vector3::dot(Vector3 vector) const
{
    return mve::dot(*this, vector);
}
Vector3 Vector3::cross(Vector3 vector) const
{
    return mve::cross(*this, vector);
}
Vector3 Vector3::reflect(Vector3 normal) const
{
    return mve::reflect(*this, normal);
}
Vector3 Vector3::inverse() const
{
    return mve::inverse(*this);
}
Vector3 Vector3::clamp_length(float min, float max) const
{
    return mve::clamp_length(*this, min, max);
}
Vector3 Vector3::round() const
{
    return mve::round(*this);
}
float Vector3::angle(Vector3 vector) const
{
    return mve::angle(*this, vector);
}
Vector3 Vector3::rotate(Vector3 axis, float angle)
{
    return mve::rotate(*this, axis, angle);
}
bool Vector3::operator!=(Vector3 other) const
{
    return x != other.x || y != other.y || z != other.z;
}
Vector3 Vector3::operator*(Vector3 other) const
{
    return Vector3(x * other.x, y * other.y, z * other.z);
}
Vector3& Vector3::operator*=(Vector3 other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;

    return *this;
}
Vector3 Vector3::operator*(float val) const
{
    return Vector3(x * val, y * val, z * val);
}
Vector3& Vector3::operator*=(float val)
{
    x *= val;
    y *= val;
    z *= val;

    return *this;
}
Vector3 Vector3::operator*(int val) const
{
    return Vector3(x * val, y * val, z * val);
}
Vector3& Vector3::operator*=(int val)
{
    x *= val;
    y *= val;
    z *= val;

    return *this;
}
Vector3 Vector3::operator+(Vector3 other) const
{
    return Vector3(x + other.x, y + other.y, z + other.z);
}
Vector3& Vector3::operator+=(Vector3 other)
{
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}
Vector3 Vector3::operator-(Vector3 other) const
{
    return Vector3(x - other.x, y - other.y, z - other.z);
}
Vector3& Vector3::operator-=(Vector3 other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;
}
Vector3 Vector3::operator/(Vector3 other) const
{
    return Vector3(x / other.x, y / other.y, z / other.z);
}
Vector3& Vector3::operator/=(Vector3 other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;

    return *this;
}
Vector3 Vector3::operator/(float val) const
{
    return Vector3(x / val, y / val, z / val);
}
Vector3& Vector3::operator/=(float val)
{
    x /= val;
    y /= val;
    z /= val;

    return *this;
}
Vector3 Vector3::operator/(int val) const
{
    return Vector3(x / val, y / val, z / val);
}
Vector3& Vector3::operator/=(int val)
{
    x /= val;
    y /= val;
    z /= val;

    return *this;
}
bool Vector3::operator<(Vector3 other) const
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
bool Vector3::operator<=(Vector3 other) const
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
bool Vector3::operator==(Vector3 other) const
{
    return x == other.x && y == other.y && z == other.z;
}
bool Vector3::operator>(Vector3 other) const
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
bool Vector3::operator>=(Vector3 other) const
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
float& Vector3::operator[](int index)
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
float& Vector3::operator[](Vector3Axis axis)
{
    switch (axis) {
    case Vector3Axis::x:
        return x;
    case Vector3Axis::y:
        return y;
    case Vector3Axis::z:
        return z;
    default:
        return x;
    }
}
Vector3 Vector3::operator+() const
{
    return Vector3(x, y, z);
}
Vector3 Vector3::operator-() const
{
    return Vector3(-x, -y, -z);
}
const float& Vector3::operator[](int index) const
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
}