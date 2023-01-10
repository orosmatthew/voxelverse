#pragma once

namespace mve {

enum class Vector3Axis { x, y, z };

class Vector3 {
public:
    float x;
    float y;
    float z;

    Vector3();

    explicit Vector3(float val);

    Vector3(float x, float y, float z);

    static Vector3 zero();

    static Vector3 one();

    [[nodiscard]] Vector3 abs() const;

    [[nodiscard]] Vector3 ceil() const;

    [[nodiscard]] Vector3 clamp(Vector3 min, Vector3 max) const;

    [[nodiscard]] Vector3 direction_to(Vector3 to) const;

    [[nodiscard]] float distance_squared_to(Vector3 to) const;

    [[nodiscard]] float distance_to(Vector3 to) const;

    [[nodiscard]] Vector3 normalize() const;

    [[nodiscard]] Vector3 floor() const;

    [[nodiscard]] float length() const;

    [[nodiscard]] float length_squared() const;

    [[nodiscard]] Vector3 linear_interpolate(Vector3 to, float weight) const;

    [[nodiscard]] Vector3Axis max_axis() const;

    [[nodiscard]] Vector3Axis min_axis() const;

    [[nodiscard]] bool is_equal_approx(Vector3 vector) const;

    [[nodiscard]] bool is_zero_approx() const;

    [[nodiscard]] Vector3 move_toward(Vector3 to, float amount) const;

    [[nodiscard]] float dot(Vector3 vector) const;

    [[nodiscard]] Vector3 cross(Vector3 vector) const;

    [[nodiscard]] Vector3 reflect(Vector3 normal) const;

    [[nodiscard]] Vector3 inverse() const;

    [[nodiscard]] Vector3 clamp_length(float min, float max) const;

    [[nodiscard]] Vector3 round() const;

    [[nodiscard]] float angle(Vector3 vector) const;

    [[nodiscard]] Vector3 rotate(Vector3 axis, float angle);

    [[nodiscard]] bool operator!=(Vector3 other) const;

    [[nodiscard]] Vector3 operator*(Vector3 other) const;

    Vector3& operator*=(Vector3 other);

    [[nodiscard]] Vector3 operator*(float val) const;

    Vector3& operator*=(float val);

    [[nodiscard]] Vector3 operator*(int val) const;

    Vector3& operator*=(int val);

    [[nodiscard]] Vector3 operator+(Vector3 other) const;

    Vector3& operator+=(Vector3 other);

    [[nodiscard]] Vector3 operator-(Vector3 other) const;

    Vector3& operator-=(Vector3 other);

    [[nodiscard]] Vector3 operator/(Vector3 other) const;

    Vector3& operator/=(Vector3 other);

    [[nodiscard]] Vector3 operator/(float val) const;

    Vector3& operator/=(float val);

    [[nodiscard]] Vector3 operator/(int val) const;

    Vector3& operator/=(int val);

    [[nodiscard]] bool operator<(Vector3 other) const;

    [[nodiscard]] bool operator<=(Vector3 other) const;

    [[nodiscard]] bool operator==(Vector3 other) const;

    [[nodiscard]] bool operator>(Vector3 other) const;

    [[nodiscard]] bool operator>=(Vector3 other) const;

    [[nodiscard]] float& operator[](int index);

    [[nodiscard]] float& operator[](Vector3Axis axis);

    [[nodiscard]] Vector3 operator+() const;

    [[nodiscard]] Vector3 operator-() const;
};

[[nodiscard]] Vector3 abs(Vector3 vector);

[[nodiscard]] Vector3 ceil(Vector3 vector);

[[nodiscard]] Vector3 clamp(Vector3 vector, Vector3 min, Vector3 max);

[[nodiscard]] Vector3 direction_to(Vector3 from, Vector3 to);

[[nodiscard]] float distance_squared_to(Vector3 from, Vector3 to);

[[nodiscard]] float distance_to(Vector3 from, Vector3 to);

[[nodiscard]] Vector3 normalize(Vector3 vector);

[[nodiscard]] Vector3 floor(Vector3 vector);

[[nodiscard]] float length(Vector3 vector);

[[nodiscard]] float length_squared(Vector3 vector);

[[nodiscard]] Vector3 linear_interpolate(Vector3 from, Vector3 to, float weight);

[[nodiscard]] Vector3Axis max_axis(Vector3 vector);

[[nodiscard]] Vector3Axis min_axis(Vector3 vector);

[[nodiscard]] bool is_equal_approx(Vector3 a, Vector3 b);

[[nodiscard]] bool is_zero_approx(Vector3 vector);

[[nodiscard]] Vector3 move_toward(Vector3 from, Vector3 to, float amount);

[[nodiscard]] float dot(Vector3 a, Vector3 b);

[[nodiscard]] Vector3 cross(Vector3 a, Vector3 b);

[[nodiscard]] Vector3 reflect(Vector3 vector, Vector3 normal);

[[nodiscard]] Vector3 inverse(Vector3 vector);

[[nodiscard]] Vector3 clamp_length(Vector3 vector, float min, float max);

[[nodiscard]] Vector3 round(Vector3 vector);

[[nodiscard]] float angle(Vector3 a, Vector3 b);

[[nodiscard]] Vector3 rotate(Vector3 vector, Vector3 axis, float angle);

}