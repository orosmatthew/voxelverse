#pragma once

namespace mve {

inline Vector2::Vector2()
    : x(0.0f)
    , y(0.0f)
{
}

inline Vector2::Vector2(const float scalar)
    : x(scalar)
    , y(scalar)
{
}

inline Vector2::Vector2(const float x, const float y)
    : x(x)
    , y(y)
{
}

inline Vector2 Vector2::abs() const
{
    return mve::abs(*this);
}

inline Vector2 Vector2::ceil() const
{
    return mve::ceil(*this);
}

inline Vector2 Vector2::clamp(const Vector2& min, const Vector2& max) const
{
    return mve::clamp(*this, min, max);
}

inline Vector2 Vector2::operator-(const Vector2& other) const
{
    return { x - other.x, y - other.y };
}

inline Vector2 Vector2::operator*(const float scalar) const
{
    return { x * scalar, y * scalar };
}

inline Vector2 Vector2::normalize() const
{
    return mve::normalize(*this);
}

inline float Vector2::direction_sqrd_to(const Vector2& to) const
{
    return distance_sqrd(*this, to);
}

inline float Vector2::distance_to(const Vector2& to) const
{
    return distance(*this, to);
}
inline float Vector2::length() const
{
    return mve::length(*this);
}
inline float Vector2::length_sqrd() const
{
    return mve::length_sqrd(*this);
}
inline Vector2 Vector2::linear_interpolate(const Vector2& to, const float weight) const
{
    return mve::linear_interpolate(*this, to, weight);
}
inline Vector2 Vector2::operator+(const Vector2& other) const
{
    return { x + other.x, y + other.y };
}
inline Vector2Axis Vector2::max_axis() const
{
    return mve::max_axis(*this);
}
inline Vector2Axis Vector2::min_axis() const
{
    return mve::min_axis(*this);
}
inline float Vector2::aspect_ratio() const
{
    return mve::aspect_ratio(*this);
}
inline Vector2 Vector2::direction_to(const Vector2& to) const
{
    return direction(*this, to);
}
inline Vector2 Vector2::floor() const
{
    return mve::floor(*this);
}
inline bool Vector2::is_equal_approx(const Vector2& vector) const
{
    return mve::is_equal_approx(*this, vector);
}
inline bool Vector2::is_zero_approx() const
{
    return mve::is_zero_approx(*this);
}
inline bool Vector2::operator!=(const Vector2& other) const
{
    return x != other.x || y != other.y;
}
inline Vector2 Vector2::operator*(const Vector2& vector) const
{
    return { x * vector.x, y * vector.y };
}
inline Vector2 Vector2::operator*(const int scalar) const
{
    return { x * static_cast<float>(scalar), y * static_cast<float>(scalar) };
}
inline Vector2 Vector2::operator/(const Vector2& other) const
{
    return { x / other.x, y / other.y };
}
inline Vector2 Vector2::operator/(const float scalar) const
{
    return { x / scalar, y / scalar };
}
inline Vector2 Vector2::operator/(const int scalar) const
{
    return { x / static_cast<float>(scalar), y / static_cast<float>(scalar) };
}
inline bool Vector2::operator<(const Vector2& other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return false;
}
inline bool Vector2::operator<=(const Vector2& other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return true;
}
inline bool Vector2::operator==(const Vector2& other) const
{
    return x == other.x && y == other.y;
}
inline bool Vector2::operator>(const Vector2& other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    return false;
}
inline bool Vector2::operator>=(const Vector2& other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    return true;
}
inline float& Vector2::operator[](const int index)
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
inline Vector2 Vector2::operator+() const
{
    return { x, y };
}
inline Vector2 Vector2::operator-() const
{
    return { -x, -y };
}
inline Vector2& Vector2::operator*=(const Vector2& vector)
{
    x *= vector.x;
    y *= vector.y;
    return *this;
}
inline Vector2& Vector2::operator-=(const Vector2& other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}
inline Vector2& Vector2::operator*=(const float scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}
inline Vector2& Vector2::operator*=(const int scalar)
{
    x *= static_cast<float>(scalar);
    y *= static_cast<float>(scalar);
    return *this;
}
inline Vector2& Vector2::operator+=(const Vector2& other)
{
    x += other.x;
    y += other.y;
    return *this;
}
inline Vector2& Vector2::operator/=(const Vector2& other)
{
    x /= other.x;
    y /= other.y;
    return *this;
}
inline Vector2& Vector2::operator/=(const float scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}
inline Vector2& Vector2::operator/=(const int scalar)
{
    x /= static_cast<float>(scalar);
    y /= static_cast<float>(scalar);
    return *this;
}
inline float& Vector2::operator[](const Vector2Axis axis)
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

inline Vector2::operator bool() const
{
    return x != 0.0f || y != 0.0f;
}
inline Vector2 Vector2::zero()
{
    return { 0.0f, 0.0f };
}
inline Vector2 Vector2::one()
{
    return { 1.0f, 1.0f };
}
inline Vector2 Vector2::move_toward(const Vector2& to, const float amount) const
{
    return mve::move_toward(*this, to, amount);
}
inline float Vector2::dot(const Vector2& vector) const
{
    return mve::dot(*this, vector);
}
inline Vector2 Vector2::reflect(const Vector2& normal) const
{
    return mve::reflect(*this, normal);
}
inline Vector2 Vector2::inverse() const
{
    return mve::inverse(*this);
}
inline Vector2 Vector2::clamp_length(const float min, const float max) const
{
    return mve::clamp_length(*this, min, max);
}
inline Vector2::Vector2(const Vector2i& vector)
    : x(static_cast<float>(vector.x))
    , y(static_cast<float>(vector.y))
{
}
inline Vector2 Vector2::rotate(const float angle) const
{
    return mve::rotate(*this, angle);
}

inline Vector2 abs(const Vector2& vector)
{
    return { abs(vector.x), abs(vector.y) };
}

inline float aspect_ratio(const Vector2& vector)
{
    return vector.x / vector.y;
}

inline Vector2 ceil(const Vector2& vector)
{
    return { ceil(vector.x), ceil(vector.y) };
}

inline Vector2 clamp(const Vector2& vector, const Vector2& min, const Vector2& max)
{
    return { clamp(vector.x, min.x, max.x), clamp(vector.y, min.y, max.y) };
}

inline Vector2 direction(const Vector2& from, const Vector2& to)
{
    return normalize(to - from);
}

inline Vector2 normalize(const Vector2& vector)
{
    Vector2 result(0.0f);
    if (const float length = vector.length(); length > 0) {
        const float inverse_length = 1.0f / length;
        result = vector * inverse_length;
    }
    return result;
}

inline float distance_sqrd(const Vector2& from, const Vector2& to)
{
    const float diff_x = to.x - from.x;
    const float diff_y = to.y - from.y;
    return sqrd(diff_x) + sqrd(diff_y);
}

inline float distance(const Vector2& from, const Vector2& to)
{
    const float diff_x = to.x - from.x;
    const float diff_y = to.y - from.y;
    return sqrt(sqrd(diff_x) + sqrd(diff_y));
}

inline Vector2 floor(const Vector2& vector)
{
    return { floor(vector.x), floor(vector.y) };
}

inline float length(const Vector2& vector)
{
    return sqrt(sqrd(vector.x) + sqrd(vector.y));
}
inline float length_sqrd(const Vector2& vector)
{
    return sqrd(vector.x) + sqrd(vector.y);
}
inline Vector2 linear_interpolate(const Vector2& from, const Vector2& to, const float weight)
{
    return from * (1.0f - weight) + to * weight;
}
inline Vector2Axis max_axis(const Vector2& vector)
{
    if (vector.x > vector.y) {
        return Vector2Axis::x;
    }
    if (vector.y > vector.x) {
        return Vector2Axis::y;
    }
    return Vector2Axis::x;
}
inline Vector2Axis min_axis(const Vector2& vector)
{
    if (vector.x < vector.y) {
        return Vector2Axis::x;
    }
    if (vector.y < vector.x) {
        return Vector2Axis::y;
    }
    return Vector2Axis::x;
}
inline bool is_equal_approx(const Vector2& a, const Vector2& b)
{
    return is_equal_approx(a.x, b.x) && is_equal_approx(a.y, b.y);
}
inline bool is_zero_approx(const Vector2& vector)
{
    return is_zero_approx(vector.x) && is_zero_approx(vector.y);
}
inline Vector2 move_toward(const Vector2& from, const Vector2& to, const float amount)
{
    if (from.distance_to(to) >= amount) {
        return to;
    }
    Vector2 result = from;
    result += from.direction_to(to) * amount;
    return result;
}
inline float dot(const Vector2& a, const Vector2& b)
{
    return a.x * b.x + a.y * b.y;
}
inline Vector2 reflect(const Vector2& vector, const Vector2& normal)
{
    const float dot = mve::dot(vector, normal);

    Vector2 result;
    result.x = vector.x - 2.0f * normal.x * dot;
    result.y = vector.y - 2.0f * normal.y * dot;

    return result;
}
inline Vector2 rotate(const Vector2& vector, const float angle)
{
    Vector2 result;
    result.x = vector.x * cos(angle) - vector.y * sin(angle);
    result.y = vector.x * sin(angle) + vector.y * cos(angle);
    return result;
}
inline Vector2 inverse(const Vector2& vector)
{
    return { 1.0f / vector.x, 1.0f / vector.y };
}
inline Vector2 clamp_length(const Vector2& vector, const float min, const float max)
{
    if (const float length_sqr = length_sqrd(vector); length_sqr > 0.0f) {
        if (const float length = sqrt(length_sqr); length < min) {
            return normalize(vector) * min;
        }
        else if (length > max) {
            return normalize(vector) * max;
        }
    }
    return vector;
}

}