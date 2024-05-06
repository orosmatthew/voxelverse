#pragma once

#include <algorithm>
#include <cmath>

namespace mve {

constexpr double epsilon = 0.00001;

constexpr double pi = 3.14159265358979323846264338327950288;

template <typename Number>
Number abs(const Number value)
{
    return std::abs(value);
}

template <typename Number>
bool is_zero_approx(const Number value, Number epsilon = mve::epsilon)
{
    return mve::abs(value) < epsilon;
}

template <typename Number>
bool is_equal_approx(const Number a, const Number b, Number epsilon = mve::epsilon)
{
    if (a == b) {
        return true;
    }
    Number tolerance = epsilon * mve::abs(a);
    if (tolerance < epsilon) {
        tolerance = epsilon;
    }
    return mve::abs(a - b) < tolerance;
}

template <typename Number>
bool approx_gte(const Number a, const Number b, Number epsilon = mve::epsilon)
{
    if (a > b) {
        return true;
    }
    Number tolerance = epsilon * mve::abs(a);
    if (tolerance < epsilon) {
        tolerance = epsilon;
    }
    return mve::abs(a - b) < tolerance;
}

template <typename Number>
bool approx_lte(const Number a, const Number b, Number epsilon = mve::epsilon)
{
    if (a < b) {
        return true;
    }
    Number tolerance = epsilon * mve::abs(a);
    if (tolerance < epsilon) {
        tolerance = epsilon;
    }
    return mve::abs(a - b) < tolerance;
}

template <typename Number>
Number ceil(const Number value)
{
    return std::ceil(value);
}

template <typename Number>
Number clamp(const Number value, const Number min, const Number max)
{
    return std::clamp(value, min, max);
}

template <typename Number>
Number sqrt(const Number value)
{
    return std::sqrt(value);
}

template <typename Number>
Number pow(const float base, const float power)
{
    return std::pow(base, power);
}

template <typename Number>
Number sqrd(const Number value)
{
    return value * value;
}

template <typename Number>
Number floor(const Number value)
{
    return std::floor(value);
}

template <typename Number>
Number lerp(const Number from, const Number to, const Number weight)
{
    return from * (1.0f - weight) + to * weight;
}

template <typename Number>
Number sin(const Number value)
{
    return std::sin(value);
}

template <typename Number>
Number cos(const Number value)
{
    return std::cos(value);
}

template <typename Number>
Number tan(const Number value)
{
    return std::tan(value);
}

template <typename Number>
Number round(const Number value)
{
    return std::round(value);
}

template <typename Number>
Number atan(const Number value)
{
    return std::atan(value);
}

template <typename Number>
Number atan2(const Number a, const Number b)
{
    return std::atan2(a, b);
}

template <typename Number>
Number radians(const Number degrees, Number pi = mve::pi)
{
    return degrees * (pi / 180.0);
}

template <typename Number>
Number degrees(const Number radians, Number pi = mve::pi)
{
    return radians * (180.0 / pi);
}

template <typename Number>
Number asin(const Number value)
{
    return std::asin(value);
}

template <typename Number>
Number acos(const Number value)
{
    return std::acos(value);
}

template <typename Number>
Number min(const Number a, const Number b)
{
    return std::min(a, b);
}

template <typename Number>
Number max(const Number a, const Number b)
{
    return std::max(a, b);
}

template <typename Number>
Number log2(const Number value)
{
    return std::log2f(value);
}

class Matrix3;
class Matrix4;
class Quaternion;
class Vector2;
class Vector2i;
class Vector3;
class Vector3i;
class Vector4;
class Vector4i;

enum class Axis4 { x, y, z, w };

class Vector4i {
public:
    int x;
    int y;
    int z;
    int w;

    Vector4i()
        : x(0)
        , y(0)
        , z(0)
        , w(0)
    {
    }

    inline explicit Vector4i(const Vector4& vector);

    Vector4i(const int x, const int y, const int z, const int w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {
    }

    static Vector4i all(const int value)
    {
        return { value, value, value, value };
    }

    static Vector4i zero()
    {
        return { 0, 0, 0, 0 };
    }

    static Vector4i one()
    {
        return { 1, 1, 1, 1 };
    }

    [[nodiscard]] Vector4i abs() const
    {
        return { mve::abs(x), mve::abs(y), mve::abs(z), mve::abs(w) };
    }

    [[nodiscard]] Vector4i clamped(const Vector4i min, const Vector4i max) const
    {
        return { clamp(x, min.x, max.x), clamp(y, min.y, max.y), clamp(z, min.z, max.z), clamp(w, min.w, max.w) };
    }

    template <typename Number>
    [[nodiscard]] Number length_sqrd() const
    {
        return sqrd(static_cast<Number>(x)) + sqrd(static_cast<Number>(y)) + sqrd(static_cast<Number>(z))
            + sqrd(static_cast<Number>(w));
    }

    template <typename Number>
    [[nodiscard]] Number length() const
    {
        return mve::sqrt(length_sqrd<Number>());
    }

    [[nodiscard]] Axis4 max_axis() const
    {
        int max_val = x;
        auto max_axis = Axis4::x;
        if (y > max_val) {
            max_val = y;
            max_axis = Axis4::y;
        }
        if (z > max_val) {
            max_val = z;
            max_axis = Axis4::z;
        }
        if (w > max_val) {
            max_axis = Axis4::w;
        }
        return max_axis;
    }

    [[nodiscard]] Axis4 min_axis() const
    {
        int min_val = x;
        auto min_axis = Axis4::x;
        if (y < min_val) {
            min_val = y;
            min_axis = Axis4::y;
        }
        if (z < min_val) {
            min_val = z;
            min_axis = Axis4::z;
        }
        if (w < min_val) {
            min_axis = Axis4::w;
        }
        return min_axis;
    }

    [[nodiscard]] bool operator!=(const Vector4i other) const
    {
        return x != other.x || y != other.y || z != other.z || w != other.w;
    }

    [[nodiscard]] Vector4i operator%(const Vector4i other) const
    {
        return { x % other.x, y & other.y, z % other.z, w % other.w };
    }

    Vector4i& operator%=(const Vector4i other)
    {
        x %= other.x;
        y %= other.y;
        z %= other.z;
        w %= other.w;
        return *this;
    }

    [[nodiscard]] Vector4i operator%(const int value) const
    {
        return { x % value, y % value, z % value, w % value };
    }

    Vector4i& operator%=(const int value)
    {
        x %= value;
        y %= value;
        z %= value;
        w %= value;
        return *this;
    }

    [[nodiscard]] Vector4i operator*(const Vector4i other) const
    {
        return { x * other.x, y * other.y, z * other.z, w * other.w };
    }

    Vector4i& operator*=(const Vector4i other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    template <typename Number>
    [[nodiscard]] Vector4i operator*(const Number value) const
    {
        return { x * static_cast<Number>(value),
                 y * static_cast<Number>(value),
                 z * static_cast<Number>(value),
                 w * static_cast<Number>(value) };
    }

    template <typename Number>
    Vector4i& operator*=(Number value)
    {
        x *= static_cast<int>(value);
        y *= static_cast<int>(value);
        z *= static_cast<int>(value);
        w *= static_cast<int>(value);
        return *this;
    }

    [[nodiscard]] Vector4i operator+(const Vector4i other) const
    {
        return { x + other.x, y + other.y, z + other.z, w + other.w };
    }

    Vector4i& operator+=(const Vector4i other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    [[nodiscard]] Vector4i operator-(const Vector4i other) const
    {
        return { x - other.x, y - other.y, z - other.z, w - other.w };
    }

    Vector4i& operator-=(const Vector4i other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    [[nodiscard]] Vector4i operator/(const Vector4i other) const
    {
        return { x / other.x, y / other.y, z / other.z, w / other.w };
    }

    Vector4i& operator/=(const Vector4i other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }

    template <typename Number>
    [[nodiscard]] Vector4i operator/(Number value) const
    {
        return { x / static_cast<Number>(value),
                 y / static_cast<Number>(value),
                 z / static_cast<Number>(value),
                 w / static_cast<Number>(value) };
    }

    template <typename Number>
    Vector4i& operator/=(Number val)
    {
        x /= static_cast<Number>(val);
        y /= static_cast<Number>(val);
        z /= static_cast<Number>(val);
        w /= static_cast<Number>(val);
        return *this;
    }

    [[nodiscard]] bool operator<(const Vector4i other) const
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

    [[nodiscard]] bool operator<=(const Vector4i other) const
    {
        return *this < other || *this == other;
    }

    [[nodiscard]] bool operator==(const Vector4i other) const
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    [[nodiscard]] bool operator>(const Vector4i other) const
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

    [[nodiscard]] bool operator>=(const Vector4i other) const
    {
        return *this > other || *this == other;
    }

    [[nodiscard]] int& operator[](const int index)
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

    [[nodiscard]] int& operator[](const Axis4 axis)
    {
        switch (axis) {
        case Axis4::x:
            return x;
        case Axis4::y:
            return y;
        case Axis4::z:
            return z;
        case Axis4::w:
            return w;
        default:
            return x;
        }
    }

    [[nodiscard]] Vector4i operator+() const
    {
        return { x, y, z, w };
    }

    [[nodiscard]] Vector4i operator-() const
    {
        return { -x, -y, -z, -w };
    }

    [[nodiscard]] explicit operator bool() const
    {
        return x != 0 || y != 0 || z != 0 || w != 0;
    }
};

enum class Vector4Axis { x, y, z, w };

class Vector4 {
public:
    float x;
    float y;
    float z;
    float w;

    inline Vector4();

    inline explicit Vector4(float val);

    inline Vector4(float x, float y, float z, float w);

    static inline Vector4 zero();

    static inline Vector4 one();

    [[nodiscard]] inline Vector4 abs() const;

    [[nodiscard]] inline Vector4 ceil() const;

    [[nodiscard]] inline Vector4 clamp(Vector4 min, Vector4 max) const;

    [[nodiscard]] inline Vector4 direction_to(Vector4 to) const;

    [[nodiscard]] inline float distance_sqrd_to(Vector4 to) const;

    [[nodiscard]] inline float distance_to(Vector4 to) const;

    [[nodiscard]] inline Vector4 normalize() const;

    [[nodiscard]] inline Vector4 floor() const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector4 linear_interpolate(Vector4 to, float weight) const;

    [[nodiscard]] inline Vector4Axis min_axis() const;

    [[nodiscard]] inline Vector4Axis max_axis() const;

    [[nodiscard]] inline bool is_equal_approx(Vector4 vector) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline float dot(Vector4 vector) const;

    [[nodiscard]] inline Vector4 inverse() const;

    [[nodiscard]] inline Vector4 clamp_length(float min, float max) const;

    [[nodiscard]] inline Vector4 round() const;

    [[nodiscard]] inline bool operator!=(Vector4 other) const;

    [[nodiscard]] inline Vector4 operator*(Vector4 other) const;

    inline Vector4& operator*=(Vector4 other);

    [[nodiscard]] inline Vector4 operator*(float val) const;

    inline Vector4& operator*=(float val);

    [[nodiscard]] inline Vector4 operator*(int val) const;

    inline Vector4& operator*=(int val);

    [[nodiscard]] inline Vector4 operator+(Vector4 other) const;

    inline Vector4& operator+=(Vector4 other);

    [[nodiscard]] inline Vector4 operator-(Vector4 other) const;

    inline Vector4& operator-=(Vector4 other);

    [[nodiscard]] inline Vector4 operator/(Vector4 other) const;

    inline Vector4& operator/=(Vector4 other);

    [[nodiscard]] inline Vector4 operator/(float val) const;

    inline Vector4& operator/=(float val);

    [[nodiscard]] inline Vector4 operator/(int val) const;

    inline Vector4& operator/=(int val);

    [[nodiscard]] inline bool operator<(Vector4 other) const;

    [[nodiscard]] inline bool operator<=(Vector4 other) const;

    [[nodiscard]] inline bool operator==(Vector4 other) const;

    [[nodiscard]] inline bool operator>(Vector4 other) const;

    [[nodiscard]] inline bool operator>=(Vector4 other) const;

    [[nodiscard]] inline float& operator[](int index);

    [[nodiscard]] inline const float& operator[](int index) const;

    [[nodiscard]] inline float& operator[](Vector4Axis axis);

    [[nodiscard]] inline Vector4 operator+() const;

    [[nodiscard]] inline Vector4 operator-() const;
};

[[nodiscard]] inline Vector4 abs(Vector4 vector);

[[nodiscard]] inline Vector4 ceil(Vector4 vector);

[[nodiscard]] inline Vector4 clamp(Vector4 vector, Vector4 min, Vector4 max);

[[nodiscard]] inline Vector4 direction(Vector4 from, Vector4 to);

[[nodiscard]] inline float distance_sqrd(Vector4 from, Vector4 to);

[[nodiscard]] inline float distance(Vector4 from, Vector4 to);

[[nodiscard]] inline Vector4 normalize(Vector4 vector);

[[nodiscard]] inline Vector4 floor(Vector4 vector);

[[nodiscard]] inline float length(Vector4 vector);

[[nodiscard]] inline float length_sqrd(Vector4 vector);

[[nodiscard]] inline Vector4 linear_interpolate(Vector4 from, Vector4 to, float weight);

[[nodiscard]] inline Vector4Axis min_axis(Vector4 vector);

[[nodiscard]] inline Vector4Axis max_axis(Vector4 vector);

[[nodiscard]] inline bool is_equal_approx(Vector4 a, Vector4 b);

[[nodiscard]] inline bool is_zero_approx(Vector4 vector);

[[nodiscard]] inline float dot(Vector4 a, Vector4 b);

[[nodiscard]] inline Vector4 inverse(Vector4 vector);

[[nodiscard]] inline Vector4 clamp_length(Vector4 vector, float min, float max);

[[nodiscard]] inline Vector4 round(Vector4 vector);

enum class Vector3iAxis { x, y, z };

class Vector3i {
public:
    int x;
    int y;
    int z;

    inline Vector3i();

    inline explicit Vector3i(Vector3 vector);

    inline Vector3i(int x, int y, int z);

    inline explicit Vector3i(int val);

    static inline Vector3i zero();

    static inline Vector3i one();

    [[nodiscard]] inline Vector3i abs() const;

    [[nodiscard]] inline Vector3i clamp(Vector3i min, Vector3i max) const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector3iAxis max_axis() const;

    [[nodiscard]] inline Vector3iAxis min_axis() const;

    [[nodiscard]] inline bool operator!=(Vector3i other) const;

    [[nodiscard]] inline Vector3i operator%(Vector3i other) const;

    inline Vector3i& operator%=(Vector3i other);

    [[nodiscard]] inline Vector3i operator%(int val) const;

    inline Vector3i& operator%=(int val);

    [[nodiscard]] inline Vector3i operator*(Vector3i other) const;

    inline Vector3i& operator*=(Vector3i other);

    [[nodiscard]] inline Vector3i operator*(float val) const;

    inline Vector3i& operator*=(float val);

    [[nodiscard]] inline Vector3i operator+(const Vector3i& other) const;

    inline Vector3i& operator+=(const Vector3i& other);

    [[nodiscard]] inline Vector3i operator-(Vector3i other) const;

    inline Vector3i& operator-=(Vector3i other);

    [[nodiscard]] inline Vector3i operator/(Vector3i other) const;

    inline Vector3i& operator/=(Vector3i other);

    [[nodiscard]] inline Vector3i operator/(float val) const;

    inline Vector3i& operator/=(float val);

    [[nodiscard]] inline Vector3i operator/(int val) const;

    inline Vector3i& operator/=(int val);

    [[nodiscard]] inline bool operator<(Vector3i other) const;

    [[nodiscard]] inline bool operator<=(Vector3i other) const;

    [[nodiscard]] inline bool operator==(Vector3i other) const;

    [[nodiscard]] inline bool operator>(Vector3i other) const;

    [[nodiscard]] inline bool operator>=(Vector3i other) const;

    [[nodiscard]] inline int& operator[](int index);

    [[nodiscard]] inline int& operator[](Vector3iAxis axis);

    [[nodiscard]] inline Vector3i operator+() const;

    [[nodiscard]] inline Vector3i operator-() const;
};

[[nodiscard]] inline Vector3i abs(Vector3i vector);

[[nodiscard]] inline Vector3i clamp(Vector3i vector, Vector3i min, Vector3i max);

[[nodiscard]] inline float length(Vector3i vector);

[[nodiscard]] inline float length_sqrd(Vector3i vector);

[[nodiscard]] inline Vector3iAxis max_axis(Vector3i vector);

[[nodiscard]] inline Vector3iAxis min_axis(Vector3i vector);

enum class Vector2iAxis { x, y };

class Vector2i {
public:
    int x;
    int y;

    inline Vector2i();

    inline explicit Vector2i(const Vector2& vector);

    inline Vector2i(int x, int y);

    inline explicit Vector2i(int val);

    static inline Vector2i zero();

    static inline Vector2i one();

    [[nodiscard]] inline Vector2i abs() const;

    [[nodiscard]] inline float aspect_ratio() const;

    [[nodiscard]] inline Vector2i clamp(const Vector2i& min, const Vector2i& max) const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector2iAxis max_axis() const;

    [[nodiscard]] inline Vector2iAxis min_axis() const;

    [[nodiscard]] inline bool operator!=(const Vector2i& other) const;

    [[nodiscard]] inline Vector2i operator%(const Vector2i& other) const;

    inline Vector2i& operator%=(const Vector2i& other);

    [[nodiscard]] inline Vector2i operator%(int val) const;

    inline Vector2i& operator%=(int val);

    [[nodiscard]] inline Vector2i operator*(const Vector2i& other) const;

    inline Vector2i& operator*=(const Vector2i& other);

    [[nodiscard]] inline Vector2i operator*(float val) const;

    inline Vector2i& operator*=(float val);

    [[nodiscard]] inline Vector2i operator*(int val) const;

    inline Vector2i& operator*=(int val);

    [[nodiscard]] inline Vector2i operator+(const Vector2i& other) const;

    inline Vector2i& operator+=(const Vector2i& other);

    [[nodiscard]] inline Vector2i operator-(const Vector2i& other) const;

    inline Vector2i& operator-=(const Vector2i& other);

    [[nodiscard]] inline Vector2i operator/(const Vector2i& other) const;

    inline Vector2i& operator/=(const Vector2i& other);

    [[nodiscard]] inline Vector2i operator/(float val) const;

    inline Vector2i& operator/=(float val);

    [[nodiscard]] inline Vector2i operator/(int val) const;

    inline Vector2i& operator/=(int val);

    [[nodiscard]] inline bool operator<(const Vector2i& other) const;

    [[nodiscard]] inline bool operator<=(const Vector2i& other) const;

    [[nodiscard]] inline bool operator==(const Vector2i& other) const;

    [[nodiscard]] inline bool operator>(const Vector2i& other) const;

    [[nodiscard]] inline bool operator>=(const Vector2i& other) const;

    [[nodiscard]] inline int& operator[](int index);

    [[nodiscard]] inline int& operator[](Vector2iAxis axis);

    [[nodiscard]] inline Vector2i operator+() const;

    [[nodiscard]] inline Vector2i operator-() const;

    [[nodiscard]] inline explicit operator bool() const;
};

[[nodiscard]] inline Vector2i abs(const Vector2i& vector);

[[nodiscard]] inline float aspect_ratio(const Vector2i& vector);

[[nodiscard]] inline Vector2i clamp(const Vector2i& vector, const Vector2i& min, const Vector2i& max);

[[nodiscard]] inline float length(const Vector2i& vector);

[[nodiscard]] inline float length_sqrd(const Vector2i& vector);

[[nodiscard]] inline Vector2iAxis max_axis(const Vector2i& vector);

[[nodiscard]] inline Vector2iAxis min_axis(const Vector2i& vector);

enum class Vector2Axis { x, y };

class Vector2 {
public:
    float x;
    float y;

    inline Vector2();

    inline explicit Vector2(const Vector2i& vector);

    inline explicit Vector2(float scalar);

    inline Vector2(float x, float y);

    static inline Vector2 zero();

    static inline Vector2 one();

    [[nodiscard]] inline Vector2 abs() const;

    [[nodiscard]] inline float aspect_ratio() const;

    [[nodiscard]] inline Vector2 ceil() const;

    [[nodiscard]] inline Vector2 clamp(const Vector2& min, const Vector2& max) const;

    [[nodiscard]] inline Vector2 direction_to(const Vector2& to) const;

    [[nodiscard]] inline float direction_sqrd_to(const Vector2& to) const;

    [[nodiscard]] inline float distance_to(const Vector2& to) const;

    [[nodiscard]] inline Vector2 normalize() const;

    [[nodiscard]] inline Vector2 floor() const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector2 linear_interpolate(const Vector2& to, float weight) const;

    [[nodiscard]] inline Vector2Axis max_axis() const;

    [[nodiscard]] inline Vector2Axis min_axis() const;

    [[nodiscard]] inline bool is_equal_approx(const Vector2& vector) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline Vector2 move_toward(const Vector2& to, float amount) const;

    [[nodiscard]] inline float dot(const Vector2& vector) const;

    [[nodiscard]] inline Vector2 reflect(const Vector2& normal) const;

    [[nodiscard]] inline Vector2 rotate(float angle) const;

    [[nodiscard]] inline Vector2 inverse() const;

    [[nodiscard]] inline Vector2 clamp_length(float min, float max) const;

    [[nodiscard]] inline bool operator!=(const Vector2& other) const;

    [[nodiscard]] inline Vector2 operator*(const Vector2& vector) const;

    inline Vector2& operator*=(const Vector2& vector);

    [[nodiscard]] inline Vector2 operator-(const Vector2& other) const;

    inline Vector2& operator-=(const Vector2& other);

    [[nodiscard]] inline Vector2 operator*(float scalar) const;

    inline Vector2& operator*=(float scalar);

    [[nodiscard]] inline Vector2 operator*(int scalar) const;

    inline Vector2& operator*=(int scalar);

    [[nodiscard]] inline Vector2 operator+(const Vector2& other) const;

    inline Vector2& operator+=(const Vector2& other);

    [[nodiscard]] inline Vector2 operator/(const Vector2& other) const;

    inline Vector2& operator/=(const Vector2& other);

    [[nodiscard]] inline Vector2 operator/(float scalar) const;

    inline Vector2& operator/=(float scalar);

    [[nodiscard]] inline Vector2 operator/(int scalar) const;

    inline Vector2& operator/=(int scalar);

    [[nodiscard]] inline bool operator<(const Vector2& other) const;

    [[nodiscard]] inline bool operator<=(const Vector2& other) const;

    [[nodiscard]] inline bool operator==(const Vector2& other) const;

    [[nodiscard]] inline bool operator>(const Vector2& other) const;

    [[nodiscard]] inline bool operator>=(const Vector2& other) const;

    [[nodiscard]] inline float& operator[](int index);

    [[nodiscard]] inline float& operator[](Vector2Axis axis);

    [[nodiscard]] inline Vector2 operator+() const;

    [[nodiscard]] inline Vector2 operator-() const;

    [[nodiscard]] inline explicit operator bool() const;
};

[[nodiscard]] inline Vector2 abs(const Vector2& vector);

[[nodiscard]] inline float aspect_ratio(const Vector2& vector);

[[nodiscard]] inline Vector2 ceil(const Vector2& vector);

[[nodiscard]] inline Vector2 clamp(const Vector2& vector, const Vector2& min, const Vector2& max);

[[nodiscard]] inline Vector2 direction(const Vector2& from, const Vector2& to);

[[nodiscard]] inline float distance_sqrd(const Vector2& from, const Vector2& to);

[[nodiscard]] inline float distance(const Vector2& from, const Vector2& to);

[[nodiscard]] inline Vector2 normalize(const Vector2& vector);

[[nodiscard]] inline Vector2 floor(const Vector2& vector);

[[nodiscard]] inline float length(const Vector2& vector);

[[nodiscard]] inline float length_sqrd(const Vector2& vector);

[[nodiscard]] inline Vector2 linear_interpolate(const Vector2& from, const Vector2& to, float weight);

[[nodiscard]] inline Vector2Axis max_axis(const Vector2& vector);

[[nodiscard]] inline Vector2Axis min_axis(const Vector2& vector);

[[nodiscard]] inline bool is_equal_approx(const Vector2& a, const Vector2& b);

[[nodiscard]] inline bool is_zero_approx(const Vector2& vector);

[[nodiscard]] inline Vector2 move_toward(const Vector2& from, const Vector2& to, float amount);

[[nodiscard]] inline float dot(const Vector2& a, const Vector2& b);

[[nodiscard]] inline Vector2 reflect(const Vector2& vector, const Vector2& normal);

[[nodiscard]] inline Vector2 rotate(const Vector2& vector, float angle);

[[nodiscard]] inline Vector2 inverse(const Vector2& vector);

[[nodiscard]] inline Vector2 clamp_length(const Vector2& vector, float min, float max);

class Quaternion {
public:
    union {
        Vector4 data;
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };

    inline Quaternion();

    inline explicit Quaternion(const Vector4& vector);

    inline Quaternion(float x, float y, float z, float w);

    [[nodiscard]] inline static Quaternion from_axis_angle(const Vector3& axis, float angle);

    [[nodiscard]] inline static Quaternion from_euler(const Vector3& euler);

    [[nodiscard]] inline static Quaternion from_matrix(const Matrix3& matrix);

    [[nodiscard]] inline static Quaternion from_vector3_to_vector3(const Vector3& from, const Vector3& to);

    [[nodiscard]] inline static Quaternion from_direction(const Vector3& dir, const Vector3& up);

    [[nodiscard]] inline float angle_to(const Quaternion& to) const;

    [[nodiscard]] inline float dot(const Quaternion& quaternion) const;

    [[nodiscard]] inline Vector3 euler() const;

    [[nodiscard]] inline Quaternion inverse() const;

    [[nodiscard]] inline bool is_equal_approx(const Quaternion& quaternion) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Quaternion spherical_linear_interpolate(const Quaternion& to, float weight) const;

    [[nodiscard]] inline Matrix3 matrix() const;

    [[nodiscard]] inline Quaternion normalize() const;

    [[nodiscard]] inline bool operator!=(const Quaternion& other) const;

    [[nodiscard]] inline Quaternion operator*(const Quaternion& other) const;

    inline void operator*=(const Quaternion& other);

    [[nodiscard]] inline Quaternion operator*(float value) const;

    inline void operator*=(float value);

    [[nodiscard]] inline Quaternion operator*(int value) const;

    inline void operator*=(int value);

    [[nodiscard]] inline Quaternion operator+(const Quaternion& other) const;

    inline void operator+=(const Quaternion& other);

    [[nodiscard]] inline Quaternion operator-(const Quaternion& other) const;

    inline void operator-=(const Quaternion& other);

    [[nodiscard]] inline Quaternion operator/(float value) const;

    inline void operator/=(float value);

    [[nodiscard]] inline Quaternion operator/(int value) const;

    inline void operator/=(int value);

    [[nodiscard]] inline bool operator==(const Quaternion& other) const;

    [[nodiscard]] inline float& operator[](int index);

    [[nodiscard]] inline const float& operator[](int index) const;

    [[nodiscard]] inline Quaternion operator+() const;

    [[nodiscard]] inline Quaternion operator-() const;
};

[[nodiscard]] inline float angle(const Quaternion& from, const Quaternion& to);

[[nodiscard]] inline float dot(const Quaternion& a, const Quaternion& b);

[[nodiscard]] inline Vector3 euler(const Quaternion& quaternion);

[[nodiscard]] inline Quaternion inverse(const Quaternion& quaternion);

[[nodiscard]] inline bool is_equal_approx(const Quaternion& a, const Quaternion& b);

[[nodiscard]] inline bool is_zero_approx(const Quaternion& quaternion);

[[nodiscard]] inline float length(const Quaternion& quaternion);

[[nodiscard]] inline float length_sqrd(const Quaternion& quaternion);

[[nodiscard]] inline Quaternion spherical_linear_interpolate(
    const Quaternion& from, const Quaternion& to, float weight);

[[nodiscard]] inline Quaternion normalize(const Quaternion& quaternion);

class Matrix4 {
public:
    Vector4 col0;
    Vector4 col1;
    Vector4 col2;
    Vector4 col3;

    inline Matrix4();

    inline Matrix4(const Vector4& col0, const Vector4& col1, const Vector4& col2, const Vector4& col3);

    [[nodiscard]] inline static Matrix4 zero();

    [[nodiscard]] inline static Matrix4 identity();

    [[nodiscard]] inline static Matrix4 from_rotation_translation(
        const Vector3& rotation_x, const Vector3& rotation_y, const Vector3& rotation_z, const Vector3& translation);

    [[nodiscard]] inline static Matrix4 from_basis_translation(const Matrix3& basis, const Vector3& translation);

    [[nodiscard]] inline float determinant() const;

    [[nodiscard]] inline float trace() const;

    [[nodiscard]] inline Matrix4 transpose() const;

    [[nodiscard]] inline Matrix4 inverse() const;

    [[nodiscard]] inline Matrix4 interpolate(const Matrix4& to, float weight) const;

    [[nodiscard]] inline Matrix4 rotate(const Vector3& axis, float angle) const;

    [[nodiscard]] inline Matrix4 rotate(const Matrix3& basis) const;

    [[nodiscard]] inline Matrix4 rotate_local(const Vector3& axis, float angle) const;

    [[nodiscard]] inline Matrix4 rotate_local(const Matrix3& basis) const;

    [[nodiscard]] inline Matrix4 scale(const Vector3& scale) const;

    [[nodiscard]] inline Matrix4 translate(const Vector3& offset) const;

    [[nodiscard]] inline Matrix4 translate_local(const Vector3& offset) const;

    [[nodiscard]] inline bool is_equal_approx(const Matrix4& matrix) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline Matrix3 basis() const;

    [[nodiscard]] inline Vector3 translation() const;

    [[nodiscard]] inline Quaternion quaternion() const;

    [[nodiscard]] inline Vector3 scale() const;

    [[nodiscard]] inline Vector3 euler() const;

    inline Vector4& operator[](int index);

    inline const Vector4& operator[](int index) const;

    [[nodiscard]] inline Matrix4 operator+(const Matrix4& other) const;

    inline void operator+=(const Matrix4& other);

    [[nodiscard]] inline Matrix4 operator-(const Matrix4& other) const;

    inline void operator-=(const Matrix4& other);

    [[nodiscard]] inline Matrix4 operator*(const Matrix4& other) const;

    inline void operator*=(const Matrix4& other);

    [[nodiscard]] inline Vector4 operator*(const Vector4& vector) const;
};

[[nodiscard]] inline float determinant(Matrix4 matrix);

[[nodiscard]] inline float trace(Matrix4 matrix);

[[nodiscard]] inline Matrix4 transpose(const Matrix4& matrix);

[[nodiscard]] inline Matrix4 inverse(Matrix4 matrix);

[[nodiscard]] inline Matrix4 interpolate(Matrix4 from, Matrix4 to, float weight);

[[nodiscard]] inline Matrix4 rotate(const Matrix4& matrix, Vector3 axis, float angle);

[[nodiscard]] inline Matrix4 rotate(const Matrix4& matrix, const Matrix3& basis);

[[nodiscard]] inline Matrix4 rotate_local(const Matrix4& matrix, const Vector3& axis, float angle);

[[nodiscard]] inline Matrix4 rotate_local(const Matrix4& matrix, const Matrix3& basis);

[[nodiscard]] inline Matrix4 scale(const Matrix4& matrix, Vector3 scale);

[[nodiscard]] inline Matrix4 translate(const Matrix4& matrix, Vector3 offset);

[[nodiscard]] inline Matrix4 translate_local(const Matrix4& matrix, Vector3 offset);

[[nodiscard]] inline bool is_equal_approx(Matrix4 a, Matrix4 b);

[[nodiscard]] inline bool is_zero_approx(Matrix4 matrix);

[[nodiscard]] inline Matrix3 basis(const Matrix4& matrix);

// [[nodiscard]] inline Matrix4 frustum(float left, float right, float bottom, float top, float near, float far);

[[nodiscard]] inline Matrix4 perspective(float fov_y, float aspect, float front, float back);

[[nodiscard]] inline Matrix4 ortho(float left, float right, float bottom, float top, float front, float back);

[[nodiscard]] inline Matrix4 look_at(Vector3 eye, Vector3 target, Vector3 up);

[[nodiscard]] inline Vector3 translation(const Matrix4& matrix);

[[nodiscard]] inline Vector3 scale(const Matrix4& matrix);

[[nodiscard]] inline Vector3 euler(const Matrix4& matrix);

enum class Vector3Axis { x, y, z };

class Vector3 {
public:
    float x;
    float y;
    float z;

    inline Vector3();

    inline explicit Vector3(Vector3i vector);

    inline explicit Vector3(float val);

    inline Vector3(float x, float y, float z);

    static inline Vector3 zero();

    static inline Vector3 one();

    [[nodiscard]] inline Vector3 abs() const;

    [[nodiscard]] inline Vector3 ceil() const;

    [[nodiscard]] inline Vector3 clamp(Vector3 min, Vector3 max) const;

    [[nodiscard]] inline Vector3 direction_to(Vector3 to) const;

    [[nodiscard]] inline float distance_sqrd_to(Vector3 to) const;

    [[nodiscard]] inline float distance_to(Vector3 to) const;

    [[nodiscard]] inline Vector3 normalize() const;

    [[nodiscard]] inline Vector3 floor() const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector3 linear_interpolate(Vector3 to, float weight) const;

    [[nodiscard]] inline Vector3Axis max_axis() const;

    [[nodiscard]] inline Vector3Axis min_axis() const;

    [[nodiscard]] inline bool is_equal_approx(Vector3 vector) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline Vector3 move_toward(Vector3 to, float amount) const;

    [[nodiscard]] inline float dot(Vector3 vector) const;

    [[nodiscard]] inline Vector3 cross(Vector3 vector) const;

    [[nodiscard]] inline Vector3 reflect(Vector3 normal) const;

    [[nodiscard]] inline Vector3 inverse() const;

    [[nodiscard]] inline Vector3 clamp_length(float min, float max) const;

    [[nodiscard]] inline Vector3 round() const;

    [[nodiscard]] inline float angle(Vector3 vector) const;

    [[nodiscard]] inline Vector3 rotate(Vector3 axis, float angle) const;

    [[nodiscard]] inline Vector3 rotate(const Matrix3& matrix) const;

    [[nodiscard]] inline Vector3 transform(const Matrix4& matrix) const;

    [[nodiscard]] inline bool operator!=(Vector3 other) const;

    [[nodiscard]] inline Vector3 operator*(Vector3 other) const;

    inline Vector3& operator*=(Vector3 other);

    [[nodiscard]] inline Vector3 operator*(float val) const;

    inline Vector3& operator*=(float val);

    [[nodiscard]] inline Vector3 operator*(int val) const;

    inline Vector3& operator*=(int val);

    [[nodiscard]] inline Vector3 operator+(Vector3 other) const;

    inline Vector3& operator+=(Vector3 other);

    [[nodiscard]] inline Vector3 operator-(const Vector3& other) const;

    inline Vector3& operator-=(const Vector3& other);

    [[nodiscard]] inline Vector3 operator/(Vector3 other) const;

    inline Vector3& operator/=(Vector3 other);

    [[nodiscard]] inline Vector3 operator/(float val) const;

    inline Vector3& operator/=(float val);

    [[nodiscard]] inline Vector3 operator/(int val) const;

    inline Vector3& operator/=(int val);

    [[nodiscard]] inline bool operator<(Vector3 other) const;

    [[nodiscard]] inline bool operator<=(Vector3 other) const;

    [[nodiscard]] inline bool operator==(Vector3 other) const;

    [[nodiscard]] inline bool operator>(Vector3 other) const;

    [[nodiscard]] inline bool operator>=(Vector3 other) const;

    [[nodiscard]] inline float& operator[](int index);

    [[nodiscard]] inline const float& operator[](int index) const;

    [[nodiscard]] inline float& operator[](Vector3Axis axis);

    [[nodiscard]] inline Vector3 operator+() const;

    [[nodiscard]] inline Vector3 operator-() const;
};

[[nodiscard]] inline Vector3 abs(Vector3 vector);

[[nodiscard]] inline Vector3 ceil(Vector3 vector);

[[nodiscard]] inline Vector3 clamp(Vector3 vector, Vector3 min, Vector3 max);

[[nodiscard]] inline Vector3 direction(Vector3 from, Vector3 to);

[[nodiscard]] inline float distance_sqrd(Vector3 from, Vector3 to);

[[nodiscard]] inline float distance(Vector3 from, Vector3 to);

[[nodiscard]] inline Vector3 normalize(Vector3 vector);

[[nodiscard]] inline Vector3 floor(Vector3 vector);

[[nodiscard]] inline float length(Vector3 vector);

[[nodiscard]] inline float length_sqrd(Vector3 vector);

[[nodiscard]] inline Vector3 linear_interpolate(Vector3 from, Vector3 to, float weight);

[[nodiscard]] inline Vector3Axis max_axis(Vector3 vector);

[[nodiscard]] inline Vector3Axis min_axis(Vector3 vector);

[[nodiscard]] inline bool is_equal_approx(Vector3 a, Vector3 b);

[[nodiscard]] inline bool is_zero_approx(Vector3 vector);

[[nodiscard]] inline Vector3 move_toward(Vector3 from, Vector3 to, float amount);

[[nodiscard]] inline float dot(Vector3 a, Vector3 b);

[[nodiscard]] inline Vector3 cross(Vector3 a, Vector3 b);

[[nodiscard]] inline Vector3 reflect(Vector3 vector, Vector3 normal);

[[nodiscard]] inline Vector3 inverse(Vector3 vector);

[[nodiscard]] inline Vector3 clamp_length(Vector3 vector, float min, float max);

[[nodiscard]] inline Vector3 round(Vector3 vector);

[[nodiscard]] inline float angle(Vector3 a, Vector3 b);

[[nodiscard]] inline Vector3 rotate(Vector3 vector, Vector3 axis, float angle);

[[nodiscard]] inline Vector3 rotate(const Vector3& vector, const Matrix3& matrix);

[[nodiscard]] inline Vector3 transform(const Vector3& position, const Matrix4& matrix);

class Matrix3 {
public:
    Vector3 col0;
    Vector3 col1;
    Vector3 col2;

    inline Matrix3();

    inline Matrix3(const Vector3& col0, const Vector3& col1, const Vector3& col2);

    inline Matrix3(
        float c0r0, float c0r1, float c0r2, float c1r0, float c1r1, float c1r2, float c2r0, float c2r1, float c2r2);

    [[nodiscard]] inline static Matrix3 from_axis_angle(const Vector3& axis, float angle);

    [[nodiscard]] inline static Matrix3 from_euler(const Vector3& euler);

    [[nodiscard]] inline static Matrix3 zero();

    [[nodiscard]] inline static Matrix3 identity();

    [[nodiscard]] inline static Matrix3 from_quaternion(const Quaternion& quaternion);

    [[nodiscard]] inline static Matrix3 from_quaternion_scale(const Quaternion& quaternion, const Vector3& scale);

    [[nodiscard]] inline static Matrix3 from_scale(const Vector3& scale);

    [[nodiscard]] inline static Matrix3 look_at(const Vector3& target, const Vector3& up);

    [[nodiscard]] inline static Matrix3 from_matrix(const Matrix4& matrix);

    [[nodiscard]] inline static Matrix3 from_direction(const Vector3& dir, const Vector3& up);

    [[nodiscard]] inline Vector3 euler() const;

    [[nodiscard]] inline Vector3 scale() const;

    [[nodiscard]] inline float determinant() const;

    [[nodiscard]] inline float trace() const;

    [[nodiscard]] inline Matrix3 transpose() const;

    [[nodiscard]] inline Matrix3 inverse() const;

    [[nodiscard]] inline Matrix3 spherical_linear_interpolate(const Matrix3& to, float weight) const;

    [[nodiscard]] inline Matrix3 rotate(const Vector3& axis, float angle) const;

    [[nodiscard]] inline Matrix3 rotate(const Quaternion& quaternion) const;

    [[nodiscard]] inline Matrix3 scale(const Vector3& scale) const;

    [[nodiscard]] inline Matrix3 orthonormalize() const;

    [[nodiscard]] inline bool is_equal_approx(const Matrix3& matrix) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline Quaternion quaternion() const;

    [[nodiscard]] inline Matrix4 with_translation(Vector3 translation) const;

    inline Vector3& operator[](int index);

    inline const Vector3& operator[](int index) const;

    [[nodiscard]] inline Matrix3 operator+(const Matrix3& other) const;

    inline void operator+=(const Matrix3& other);

    [[nodiscard]] inline Matrix3 operator-(const Matrix3& other) const;

    inline void operator-=(const Matrix3& other);

    [[nodiscard]] inline Matrix3 operator*(const Matrix3& other) const;

    inline void operator*=(const Matrix3& other);

    [[nodiscard]] inline Matrix3 operator*(float val) const;

    inline void operator*=(float val);

    [[nodiscard]] inline Matrix3 operator*(int val) const;

    inline void operator*=(int val);

    [[nodiscard]] inline Vector3 operator*(Vector3 vector) const;

    [[nodiscard]] inline bool operator!=(const Matrix3& other) const;

    [[nodiscard]] inline bool operator==(const Matrix3& other) const;

    [[nodiscard]] inline bool operator<(const Matrix3& other) const;

    [[nodiscard]] inline bool operator<=(const Matrix3& other) const;

    [[nodiscard]] inline bool operator>(const Matrix3& other) const;

    [[nodiscard]] inline bool operator>=(const Matrix3& other) const;
};

[[nodiscard]] inline Vector3 euler(const Matrix3& matrix);

[[nodiscard]] inline Vector3 scale(const Matrix3& matrix);

[[nodiscard]] inline float determinant(const Matrix3& matrix);

[[nodiscard]] inline float trace(const Matrix3& matrix);

[[nodiscard]] inline Matrix3 transpose(const Matrix3& matrix);

[[nodiscard]] inline Matrix3 inverse(const Matrix3& matrix);

[[nodiscard]] inline Matrix3 spherical_linear_interpolate(const Matrix3& from, const Matrix3& to, float weight);

[[nodiscard]] inline Matrix3 rotate(const Matrix3& matrix, const Vector3& axis, float angle);

[[nodiscard]] inline Matrix3 rotate(const Matrix3& matrix, const Quaternion& quaternion);

[[nodiscard]] inline Matrix3 scale(const Matrix3& matrix, const Vector3& scale);

[[nodiscard]] inline Matrix3 orthonormalize(const Matrix3& matrix);

[[nodiscard]] inline bool is_equal_approx(const Matrix3& a, const Matrix3& b);

[[nodiscard]] inline bool is_zero_approx(const Matrix3& matrix);

[[nodiscard]] inline Matrix4 with_translation(const Matrix3& matrix, Vector3 translation);

[[nodiscard]] inline Matrix3 look_at(const Vector3& target, const Vector3& up);

}

#include "detail/matrix3.inl"
#include "detail/matrix4.inl"
#include "detail/quaternion.inl"
#include "detail/vector2.inl"
#include "detail/vector2i.inl"
#include "detail/vector3.inl"
#include "detail/vector3i.inl"
#include "detail/vector4.inl"
#include "detail/vector4i.inl"