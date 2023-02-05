#pragma once

#include "quaternion.hpp"
#include "vector3.hpp"

namespace mve {

class Matrix4;

class Matrix3 {
public:
    Vector3 col0;
    Vector3 col1;
    Vector3 col2;

    Matrix3();

    Matrix3(const Vector3& col0, const Vector3& col1, const Vector3& col2);

    Matrix3(float c0r0, float c0r1, float c0r2, float c1r0, float c1r1, float c1r2, float c2r0, float c2r1, float c2r2);

    [[nodiscard]] static Matrix3 from_axis_angle(const Vector3& axis, float angle);

    [[nodiscard]] static Matrix3 from_euler(const Vector3& euler);

    [[nodiscard]] static Matrix3 zero();

    [[nodiscard]] static Matrix3 identity();

    [[nodiscard]] static Matrix3 from_quaternion(const Quaternion& quaternion);

    [[nodiscard]] static Matrix3 from_quaternion_scale(const Quaternion& quaternion, const Vector3& scale);

    [[nodiscard]] static Matrix3 from_scale(const Vector3& scale);

    [[nodiscard]] static Matrix3 look_at(const Vector3& target, const Vector3& up);

    [[nodiscard]] static Matrix3 from_matrix(const Matrix4& matrix);

    [[nodiscard]] Vector3 euler() const;

    [[nodiscard]] Vector3 scale() const;

    [[nodiscard]] float determinant() const;

    [[nodiscard]] float trace() const;

    [[nodiscard]] Matrix3 transpose() const;

    [[nodiscard]] Matrix3 inverse() const;

    [[nodiscard]] Matrix3 spherical_linear_interpolate(const Matrix3& to, float weight) const;

    [[nodiscard]] Matrix3 rotate(const Vector3& axis, float angle) const;

    [[nodiscard]] Matrix3 rotate(const Quaternion& quaternion) const;

    [[nodiscard]] Matrix3 scale(const Vector3& scale) const;

    [[nodiscard]] Matrix3 orthonormalize() const;

    [[nodiscard]] bool is_equal_approx(const Matrix3& matrix) const;

    [[nodiscard]] bool is_zero_approx() const;

    [[nodiscard]] Quaternion quaternion() const;

    [[nodiscard]] Matrix4 with_translation(const Vector3 translation) const;

    Vector3& operator[](int index);

    const Vector3& operator[](int index) const;

    [[nodiscard]] Matrix3 operator+(const Matrix3& other) const;

    void operator+=(const Matrix3& other);

    [[nodiscard]] Matrix3 operator-(const Matrix3& other) const;

    void operator-=(const Matrix3& other);

    [[nodiscard]] Matrix3 operator*(const Matrix3& other) const;

    void operator*=(const Matrix3& other);

    [[nodiscard]] Matrix3 operator*(float val) const;

    void operator*=(float val);

    [[nodiscard]] Matrix3 operator*(int val) const;

    void operator*=(int val);

    [[nodiscard]] Vector3 operator*(Vector3 vector) const;

    [[nodiscard]] bool operator!=(const Matrix3& other) const;

    [[nodiscard]] bool operator==(const Matrix3& other) const;

    [[nodiscard]] bool operator<(const Matrix3& other) const;

    [[nodiscard]] bool operator<=(const Matrix3& other) const;

    [[nodiscard]] bool operator>(const Matrix3& other) const;

    [[nodiscard]] bool operator>=(const Matrix3& other) const;
};

[[nodiscard]] Vector3 euler(const Matrix3& matrix);

[[nodiscard]] Vector3 scale(const Matrix3& matrix);

[[nodiscard]] float determinant(const Matrix3& matrix);

[[nodiscard]] float trace(const Matrix3& matrix);

[[nodiscard]] Matrix3 transpose(const Matrix3& matrix);

[[nodiscard]] Matrix3 inverse(const Matrix3& matrix);

[[nodiscard]] Matrix3 spherical_linear_interpolate(const Matrix3& from, const Matrix3& to, float weight);

[[nodiscard]] Matrix3 rotate(const Matrix3& matrix, const Vector3& axis, float angle);

[[nodiscard]] Matrix3 rotate(const Matrix3& matrix, const Quaternion& quaternion);

[[nodiscard]] Matrix3 scale(const Matrix3& matrix, const Vector3& scale);

[[nodiscard]] Matrix3 orthonormalize(const Matrix3& matrix);

[[nodiscard]] bool is_equal_approx(const Matrix3& a, const Matrix3& b);

[[nodiscard]] bool is_zero_approx(const Matrix3& matrix);

[[nodiscard]] Matrix4 with_translation(const Matrix3& matrix, const Vector3 translation);

[[nodiscard]] Matrix3 look_at(const Vector3& target, const Vector3& up);

}