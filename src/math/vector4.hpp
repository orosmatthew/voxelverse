#pragma once

namespace mve {

enum class Vector4Axis { x, y, z, w };

class Vector4 {
public:
    float x;
    float y;
    float z;
    float w;

    Vector4();

    explicit Vector4(float val);

    Vector4(float x, float y, float z, float w);

    static Vector4 zero();

    static Vector4 one();

    [[nodiscard]] Vector4 abs() const;

    [[nodiscard]] Vector4 ceil() const;

    [[nodiscard]] Vector4 clamp(Vector4 min, Vector4 max) const;

    [[nodiscard]] Vector4 direction_to(Vector4 to) const;

    [[nodiscard]] float distance_squared_to(Vector4 to) const;

    [[nodiscard]] float distance_to(Vector4 to) const;

    [[nodiscard]] Vector4 normalize() const;

    [[nodiscard]] Vector4 floor() const;

    [[nodiscard]] float length() const;

    [[nodiscard]] float length_squared() const;

    [[nodiscard]] Vector4 linear_interpolate(Vector4 to, float weight) const;

    [[nodiscard]] Vector4Axis min_axis() const;

    [[nodiscard]] Vector4Axis max_axis() const;

    [[nodiscard]] bool is_equal_approx(Vector4 vector) const;

    [[nodiscard]] bool is_zero_approx() const;

    [[nodiscard]] float dot(Vector4 vector) const;

    [[nodiscard]] Vector4 inverse() const;

    [[nodiscard]] Vector4 clamp_length(float min, float max) const;

    [[nodiscard]] Vector4 round() const;

    [[nodiscard]] bool operator!=(Vector4 other) const;

    [[nodiscard]] Vector4 operator*(Vector4 other) const;

    Vector4& operator*=(Vector4 other);

    [[nodiscard]] Vector4 operator*(float val) const;

    Vector4& operator*=(float val);

    [[nodiscard]] Vector4 operator*(int val) const;

    Vector4& operator*=(int val);

    [[nodiscard]] Vector4 operator+(Vector4 other) const;

    Vector4& operator+=(Vector4 other);

    [[nodiscard]] Vector4 operator-(Vector4 other) const;

    Vector4& operator-=(Vector4 other);

    [[nodiscard]] Vector4 operator/(Vector4 other) const;

    Vector4& operator/=(Vector4 other);

    [[nodiscard]] Vector4 operator/(float val) const;

    Vector4& operator/=(float val);

    [[nodiscard]] Vector4 operator/(int val) const;

    Vector4& operator/=(int val);

    [[nodiscard]] bool operator<(Vector4 other) const;

    [[nodiscard]] bool operator<=(Vector4 other) const;

    [[nodiscard]] bool operator==(Vector4 other) const;

    [[nodiscard]] bool operator>(Vector4 other) const;

    [[nodiscard]] bool operator>=(Vector4 other) const;

    [[nodiscard]] float& operator[](int index);

    [[nodiscard]] float& operator[](Vector4Axis axis);

    [[nodiscard]] Vector4 operator+() const;

    [[nodiscard]] Vector4 operator-() const;
};

[[nodiscard]] Vector4 abs(Vector4 vector);

[[nodiscard]] Vector4 ceil(Vector4 vector);

[[nodiscard]] Vector4 clamp(Vector4 vector, Vector4 min, Vector4 max);

[[nodiscard]] Vector4 direction_to(Vector4 from, Vector4 to);

[[nodiscard]] float distance_squared_to(Vector4 from, Vector4 to);

[[nodiscard]] float distance_to(Vector4 from, Vector4 to);

[[nodiscard]] Vector4 normalize(Vector4 vector);

[[nodiscard]] Vector4 floor(Vector4 vector);

[[nodiscard]] float length(Vector4 vector);

[[nodiscard]] float length_squared(Vector4 vector);

[[nodiscard]] Vector4 linear_interpolate(Vector4 from, Vector4 to, float weight);

[[nodiscard]] Vector4Axis min_axis(Vector4 vector);

[[nodiscard]] Vector4Axis max_axis(Vector4 vector);

[[nodiscard]] bool is_equal_approx(Vector4 a, Vector4 b);

[[nodiscard]] bool is_zero_approx(Vector4 vector);

[[nodiscard]] float dot(Vector4 a, Vector4 b);

[[nodiscard]] Vector4 inverse(Vector4 vector);

[[nodiscard]] Vector4 clamp_length(Vector4 vector, float min, float max);

[[nodiscard]] Vector4 round(Vector4 vector);

}