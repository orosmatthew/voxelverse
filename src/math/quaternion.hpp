#pragma once

#include "vector4.hpp"

namespace mve {

class Vector3;
class Matrix3;

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

    Quaternion();

    Quaternion(const Vector4& vector);

    Quaternion(float x, float y, float z, float w);

    [[nodiscard]] static Quaternion from_axis_angle(const Vector3& axis, float angle);

    [[nodiscard]] static Quaternion from_euler(const Vector3& euler);

    [[nodiscard]] static Quaternion from_matrix(const Matrix3& matrix);

    [[nodiscard]] float angle_to(const Quaternion& to) const;

    [[nodiscard]] float dot(const Quaternion& quaternion) const;

    [[nodiscard]] Vector3 euler() const;

    [[nodiscard]] Quaternion inverse() const;

    [[nodiscard]] bool is_equal_approx(const Quaternion& quaternion) const;

    [[nodiscard]] bool is_zero_approx() const;

    [[nodiscard]] float length() const;

    [[nodiscard]] float length_squared() const;

    [[nodiscard]] Quaternion spherical_linear_interpolate(const Quaternion& to, float weight) const;

    [[nodiscard]] Matrix3 matrix() const;

    [[nodiscard]] Quaternion normalize() const;

    [[nodiscard]] bool operator!=(const Quaternion& other) const;

    [[nodiscard]] Quaternion operator*(const Quaternion& other) const;

    void operator*=(const Quaternion& other);

    [[nodiscard]] Quaternion operator*(float value) const;

    void operator*=(float value);

    [[nodiscard]] Quaternion operator*(int value) const;

    void operator*=(int value);

    [[nodiscard]] Quaternion operator+(const Quaternion& other) const;

    void operator+=(const Quaternion& other);

    [[nodiscard]] Quaternion operator-(const Quaternion& other) const;

    void operator-=(const Quaternion& other);

    [[nodiscard]] Quaternion operator/(float value) const;

    void operator/=(float value);

    [[nodiscard]] Quaternion operator/(int value) const;

    void operator/=(int value);

    [[nodiscard]] bool operator==(const Quaternion& other) const;

    [[nodiscard]] float& operator[](int index);

    [[nodiscard]] const float& operator[](int index) const;

    [[nodiscard]] Quaternion operator+() const;

    [[nodiscard]] Quaternion operator-() const;
};

[[nodiscard]] float angle_to(const Quaternion& from, const Quaternion& to);

[[nodiscard]] float dot(const Quaternion& a, const Quaternion& b);

[[nodiscard]] Vector3 euler(const Quaternion& quaternion);

[[nodiscard]] Quaternion inverse(const Quaternion& quaternion);

[[nodiscard]] bool is_equal_approx(const Quaternion& a, const Quaternion& b);

[[nodiscard]] bool is_zero_approx(const Quaternion& quaternion);

[[nodiscard]] float length(const Quaternion& quaternion);

[[nodiscard]] float length_squared(const Quaternion& quaternion);

[[nodiscard]] Quaternion spherical_linear_interpolate(const Quaternion& from, const Quaternion& to, float weight);

[[nodiscard]] Quaternion normalize(const Quaternion& quaternion);

}