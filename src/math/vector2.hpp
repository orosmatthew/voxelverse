#pragma once

namespace mve {

enum class Vector2Axis { x, y };

class Vector2 {
public:
    float x;
    float y;

    Vector2();

    explicit Vector2(float scalar);

    Vector2(float x, float y);

    static Vector2 zero();

    static Vector2 one();

    [[nodiscard]] Vector2 abs() const;

    [[nodiscard]] float aspect_ratio() const;

    [[nodiscard]] Vector2 ceil() const;

    [[nodiscard]] Vector2 clamp(const Vector2& min, const Vector2& max) const;

    [[nodiscard]] Vector2 direction_to(const Vector2& to) const;

    [[nodiscard]] float distance_squared_to(const Vector2& to) const;

    [[nodiscard]] float distance_to(const Vector2& to) const;

    [[nodiscard]] Vector2 normalize() const;

    [[nodiscard]] Vector2 floor() const;

    [[nodiscard]] float length() const;

    [[nodiscard]] float length_squared() const;

    [[nodiscard]] Vector2 linear_interpolate(const Vector2& to, float weight) const;

    [[nodiscard]] Vector2Axis max_axis() const;

    [[nodiscard]] Vector2Axis min_axis() const;

    [[nodiscard]] bool is_equal_approx(const Vector2& vector) const;

    [[nodiscard]] bool is_zero_approx() const;

    [[nodiscard]] Vector2 move_toward(const Vector2& to, float amount) const;

    [[nodiscard]] float dot(const Vector2& vector) const;

    [[nodiscard]] Vector2 reflect(const Vector2& normal) const;

    [[nodiscard]] Vector2 inverse() const;

    [[nodiscard]] Vector2 clamp_length(float min, float max) const;

    [[nodiscard]] bool operator!=(const Vector2& other) const;

    [[nodiscard]] Vector2 operator*(const Vector2& vector) const;

    Vector2& operator*=(const Vector2& vector);

    [[nodiscard]] Vector2 operator-(const Vector2& other) const;

    Vector2& operator-=(const Vector2& other);

    [[nodiscard]] Vector2 operator*(float scalar) const;

    Vector2& operator*=(float scalar);

    [[nodiscard]] Vector2 operator*(int scalar) const;

    Vector2& operator*=(int scalar);

    [[nodiscard]] Vector2 operator+(const Vector2& other) const;

    Vector2& operator+=(const Vector2& other);

    [[nodiscard]] Vector2 operator/(const Vector2& other) const;

    Vector2& operator/=(const Vector2& other);

    [[nodiscard]] Vector2 operator/(float scalar) const;

    Vector2& operator/=(float scalar);

    [[nodiscard]] Vector2 operator/(int scalar) const;

    Vector2& operator/=(int scalar);

    [[nodiscard]] bool operator<(const Vector2& other) const;

    [[nodiscard]] bool operator<=(const Vector2& other) const;

    [[nodiscard]] bool operator==(const Vector2& other) const;

    [[nodiscard]] bool operator>(const Vector2& other) const;

    [[nodiscard]] bool operator>=(const Vector2& other) const;

    [[nodiscard]] float operator[](int index) const;

    [[nodiscard]] float operator[](Vector2Axis axis) const;

    [[nodiscard]] Vector2 operator+() const;

    [[nodiscard]] Vector2 operator-() const;

    [[nodiscard]] operator bool() const;
};

[[nodiscard]] Vector2 abs(const Vector2& vector);

[[nodiscard]] float aspect_ratio(const Vector2& vector);

[[nodiscard]] Vector2 ceil(const Vector2& vector);

[[nodiscard]] Vector2 clamp(const Vector2& vector, const Vector2& min, const Vector2& max);

[[nodiscard]] Vector2 direction_to(const Vector2& from, const Vector2& to);

[[nodiscard]] float distance_squared_to(const Vector2& from, const Vector2& to);

[[nodiscard]] float distance_to(const Vector2& from, const Vector2& to);

[[nodiscard]] Vector2 normalize(const Vector2& vector);

[[nodiscard]] Vector2 floor(const Vector2& vector);

[[nodiscard]] float length(const Vector2& vector);

[[nodiscard]] float length_squared(const Vector2& vector);

[[nodiscard]] Vector2 linear_interpolate(const Vector2& from, const Vector2& to, float weight);

[[nodiscard]] Vector2Axis max_axis(const Vector2& vector);

[[nodiscard]] Vector2Axis min_axis(const Vector2& vector);

[[nodiscard]] bool is_equal_approx(const Vector2& a, const Vector2& b);

[[nodiscard]] bool is_zero_approx(const Vector2& vector);

[[nodiscard]] Vector2 move_toward(const Vector2& from, const Vector2& to, float amount);

[[nodiscard]] float dot(const Vector2& a, const Vector2& b);

[[nodiscard]] Vector2 reflect(const Vector2& vector, const Vector2& normal);

[[nodiscard]] Vector2 rotate(const Vector2& vector, float angle);

[[nodiscard]] Vector2 inverse(const Vector2& vector);

[[nodiscard]] Vector2 clamp_length(const Vector2& vector, float min, float max);

}