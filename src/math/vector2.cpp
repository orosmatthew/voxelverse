#include "vector2.hpp"

#include "functions.hpp"

namespace mve {

Vector2::Vector2()
    : x(0.0f)
    , y(0.0f)
{
}

Vector2::Vector2(float scalar)
    : x(scalar)
    , y(scalar)
{
}

Vector2::Vector2(float x, float y)
    : x(x)
    , y(y)
{
}

Vector2 Vector2::abs() const
{
    return mve::abs(*this);
}

Vector2 Vector2::ceil() const
{
    return mve::ceil(*this);
}

Vector2 Vector2::clamp(const Vector2& min, const Vector2& max) const
{
    return mve::clamp(*this, min, max);
}

Vector2 Vector2::operator-(const Vector2& other) const
{
    return Vector2(x - other.x, y - other.y);
}

Vector2 Vector2::operator*(float scalar) const
{
    return Vector2(x * scalar, y * scalar);
}

Vector2 Vector2::normalize() const
{
    return mve::normalize(*this);
}

float Vector2::distance_squared_to(const Vector2& to) const
{
    return mve::distance_squared_to(*this, to);
}

float Vector2::distance_to(const Vector2& to) const
{
    return mve::distance_to(*this, to);
}
float Vector2::length() const
{
    return mve::length(*this);
}
float Vector2::length_squared() const
{
    return mve::length_squared(*this);
}
Vector2 Vector2::linear_interpolate(const Vector2& to, float weight) const
{
    return mve::linear_interpolate(*this, to, weight);
}
Vector2 Vector2::operator+(const Vector2& other) const
{
    return Vector2(x + other.x, y + other.y);
}
Vector2Axis Vector2::max_axis() const
{
    return mve::max_axis(*this);
}
Vector2Axis Vector2::min_axis() const
{
    return mve::min_axis(*this);
}
float Vector2::aspect_ratio() const
{
    return mve::aspect_ratio(*this);
}
Vector2 Vector2::direction_to(const Vector2& to) const
{
    return mve::direction_to(*this, to);
}
Vector2 Vector2::floor() const
{
    return mve::floor(*this);
}
bool Vector2::is_equal_approx(const Vector2& vector) const
{
    return mve::is_equal_approx(*this, vector);
}
bool Vector2::is_zero_approx() const
{
    return mve::is_zero_approx(*this);
}
bool Vector2::operator!=(const Vector2& other) const
{
    return x != other.x || y != other.y;
}
Vector2 Vector2::operator*(const Vector2& vector) const
{
    return Vector2(x * vector.x, y * vector.y);
}
Vector2 Vector2::operator*(int scalar) const
{
    return Vector2(x * scalar, y * scalar);
}
Vector2 Vector2::operator/(const Vector2& other) const
{
    return Vector2(x / other.x, y / other.y);
}
Vector2 Vector2::operator/(float scalar) const
{
    return Vector2(x / scalar, y / scalar);
}
Vector2 Vector2::operator/(int scalar) const
{
    return Vector2(x / scalar, y / scalar);
}
bool Vector2::operator<(const Vector2& other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return false;
}
bool Vector2::operator<=(const Vector2& other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return true;
}
bool Vector2::operator==(const Vector2& other) const
{
    return x == other.x && y == other.y;
}
bool Vector2::operator>(const Vector2& other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    return false;
}
bool Vector2::operator>=(const Vector2& other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    return true;
}
float Vector2::operator[](int index) const
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
Vector2 Vector2::operator+() const
{
    return Vector2(x, y);
}
Vector2 Vector2::operator-() const
{
    return Vector2(-x, -y);
}
Vector2& Vector2::operator*=(const Vector2& vector)
{
    x *= vector.x;
    y *= vector.y;
    return *this;
}
Vector2& Vector2::operator-=(const Vector2& other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}
Vector2& Vector2::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}
Vector2& Vector2::operator*=(int scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}
Vector2& Vector2::operator+=(const Vector2& other)
{
    x += other.x;
    y += other.y;
    return *this;
}
Vector2& Vector2::operator/=(const Vector2& other)
{
    x /= other.x;
    y /= other.y;
    return *this;
}
Vector2& Vector2::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}
Vector2& Vector2::operator/=(int scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}
float Vector2::operator[](Vector2Axis axis) const
{
    switch (axis) {
    case Vector2Axis::x:
        return x;
    case Vector2Axis::y:
        return y;
    default:
        return x;
    }
}

Vector2::operator bool() const
{
    return x != 0.0f || y != 0.0f;
}
Vector2 Vector2::zero()
{
    return Vector2(0.0f, 0.0f);
}
Vector2 Vector2::one()
{
    return Vector2(1.0f, 1.0f);
}
Vector2 Vector2::move_toward(const Vector2& to, float amount) const
{
    return mve::move_toward(*this, to, amount);
}
float Vector2::dot(const Vector2& vector) const
{
    return mve::dot(*this, vector);
}
Vector2 Vector2::reflect(const Vector2& normal) const
{
    return mve::reflect(*this, normal);
}
Vector2 Vector2::inverse() const
{
    return mve::inverse(*this);
}
Vector2 Vector2::clamp_length(float min, float max) const
{
    return mve::clamp_length(*this, min, max);
}

Vector2 abs(const Vector2& vector)
{
    return Vector2(abs(vector.x), abs(vector.y));
}

float aspect_ratio(const Vector2& vector)
{
    return vector.x / vector.y;
}

Vector2 ceil(const Vector2& vector)
{
    return Vector2(ceil(vector.x), ceil(vector.y));
}

Vector2 clamp(const Vector2& vector, const Vector2& min, const Vector2& max)
{
    return Vector2(clamp(vector.x, min.x, max.x), clamp(vector.y, min.y, max.y));
}

Vector2 direction_to(const Vector2& from, const Vector2& to)
{
    return normalize(to - from);
}

Vector2 normalize(const Vector2& vector)
{
    Vector2 result(0.0f);
    float length = vector.length();

    if (length > 0) {
        float inverse_length = 1.0f / length;
        result = vector * inverse_length;
    }

    return result;
}

float distance_squared_to(const Vector2& from, const Vector2& to)
{
    float diff_x = to.x - from.x;
    float diff_y = to.y - from.y;
    return squared(diff_x) + squared(diff_y);
}

float distance_to(const Vector2& from, const Vector2& to)
{
    float diff_x = to.x - from.x;
    float diff_y = to.y - from.y;
    return sqrt(squared(diff_x) + squared(diff_y));
}

Vector2 floor(const Vector2& vector)
{
    return Vector2(floor(vector.x), floor(vector.y));
}

float length(const Vector2& vector)
{
    return sqrt(squared(vector.x) + squared(vector.y));
}
float length_squared(const Vector2& vector)
{
    return squared(vector.x) + squared(vector.y);
}
Vector2 linear_interpolate(const Vector2& from, const Vector2& to, float weight)
{
    return (from * (1.0f - weight)) + (to * weight);
}
Vector2Axis max_axis(const Vector2& vector)
{
    if (vector.x > vector.y) {
        return Vector2Axis::x;
    }
    else if (vector.y > vector.x) {
        return Vector2Axis::y;
    }
    return Vector2Axis::x;
}
Vector2Axis min_axis(const Vector2& vector)
{
    if (vector.x < vector.y) {
        return Vector2Axis::x;
    }
    else if (vector.y < vector.x) {
        return Vector2Axis::y;
    }
    return Vector2Axis::x;
}
bool is_equal_approx(const Vector2& a, const Vector2& b)
{
    return is_equal_approx(a.x, b.x) && is_equal_approx(a.y, b.y);
}
bool is_zero_approx(const Vector2& vector)
{
    return is_zero_approx(vector.x) && is_zero_approx(vector.y);
}
Vector2 move_toward(const Vector2& from, const Vector2& to, float amount)
{
    if (from.distance_to(to) >= amount) {
        return to;
    }
    Vector2 result = from;
    result += from.direction_to(to) * amount;
    return result;
}
float dot(const Vector2& a, const Vector2& b)
{
    return (a.x * b.x + a.y * b.y);
}
Vector2 reflect(const Vector2& vector, const Vector2& normal)
{
    float dot = mve::dot(vector, normal);

    Vector2 result;
    result.x = vector.x - (2.0f * normal.x) * dot;
    result.y = vector.y - (2.0f * normal.y) * dot;

    return result;
}
Vector2 rotate(const Vector2& vector, float angle)
{
    Vector2 result;
    result.x = vector.x * cos(angle) - vector.y * sin(angle);
    result.y = vector.x * sin(angle) + vector.y * cos(angle);
    return result;
}
Vector2 inverse(const Vector2& vector)
{
    return Vector2(1.0f / vector.x, 1.0f / vector.y);
}
Vector2 clamp_length(const Vector2& vector, float min, float max)
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

}