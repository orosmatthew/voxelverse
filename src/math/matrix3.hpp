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

    Matrix3(Vector3 col0, Vector3 col1, Vector3 col2);

    Matrix3(float c0r0, float c0r1, float c0r2, float c1r0, float c1r1, float c1r2, float c2r0, float c2r1, float c2r2);

    [[nodiscard]] static Matrix3 from_axis_angle(Vector3 axis, float angle);

    [[nodiscard]] static Matrix3 from_euler(Vector3 euler);

    [[nodiscard]] static Matrix3 zero();

    [[nodiscard]] static Matrix3 identity();

    [[nodiscard]] static Matrix3 from_quaternion(const Quaternion& quaternion);

    [[nodiscard]] static Matrix3 from_quaternion_scale(const Quaternion& quaternion, const Vector3& scale);

    [[nodiscard]] static Matrix3 look_at(const Vector3 target, const Vector3 up);

    [[nodiscard]] static Matrix3 from_matrix(const Matrix4& matrix);

    [[nodiscard]] Vector3 euler() const;

    [[nodiscard]] Vector3 scale() const;

    [[nodiscard]] float determinant() const;

    [[nodiscard]] float trace() const;

    [[nodiscard]] Matrix3 transpose() const;

    [[nodiscard]] Matrix3 inverse() const;

    [[nodiscard]] Matrix3 spherical_linear_interpolate(Matrix3 to, float weight) const;

    [[nodiscard]] Matrix3 rotate(Vector3 axis, float angle) const;

    [[nodiscard]] Matrix3 rotate(const Quaternion& quaternion) const;

    [[nodiscard]] Matrix3 scale(Vector3 scale) const;

    [[nodiscard]] Matrix3 orthonormalize() const;

    [[nodiscard]] bool is_equal_approx(Matrix3 matrix) const;

    [[nodiscard]] bool is_zero_approx() const;

    [[nodiscard]] Quaternion quaternion() const;

    Vector3& operator[](int index);

    const Vector3& operator[](int index) const;

    [[nodiscard]] Matrix3 operator+(Matrix3 other) const;

    Matrix3& operator+=(Matrix3 other);

    [[nodiscard]] Matrix3 operator-(Matrix3 other) const;

    Matrix3& operator-=(Matrix3 other);

    [[nodiscard]] Matrix3 operator*(Matrix3 other) const;

    Matrix3& operator*=(Matrix3 other);

    [[nodiscard]] Matrix3 operator*(float val) const;

    Matrix3& operator*=(float val);

    [[nodiscard]] Matrix3 operator*(int val) const;

    Matrix3& operator*=(int val);

    [[nodiscard]] bool operator!=(Matrix3 other) const;

    [[nodiscard]] bool operator==(Matrix3 other) const;

    [[nodiscard]] bool operator<(Matrix3 other) const;

    [[nodiscard]] bool operator<=(Matrix3 other) const;

    [[nodiscard]] bool operator>(Matrix3 other) const;

    [[nodiscard]] bool operator>=(Matrix3 other) const;
};

[[nodiscard]] Vector3 euler(Matrix3 matrix);

[[nodiscard]] Vector3 scale(Matrix3 matrix);

[[nodiscard]] float determinant(Matrix3 matrix);

[[nodiscard]] float trace(Matrix3 matrix);

[[nodiscard]] Matrix3 transpose(Matrix3 matrix);

[[nodiscard]] Matrix3 inverse(Matrix3 matrix);

[[nodiscard]] Matrix3 spherical_linear_interpolate(Matrix3 from, Matrix3 to, float weight);

[[nodiscard]] Matrix3 rotate(Matrix3 matrix, Vector3 axis, float angle);

[[nodiscard]] Matrix3 rotate(const Matrix3& matrix, const Quaternion& quaternion);

[[nodiscard]] Matrix3 scale(const Matrix3& matrix, const Vector3& scale);

[[nodiscard]] Matrix3 orthonormalize(const Matrix3& matrix);

[[nodiscard]] bool is_equal_approx(Matrix3 a, Matrix3 b);

[[nodiscard]] bool is_zero_approx(Matrix3 matrix);

[[nodiscard]] Quaternion quaternion(const Matrix3& matrix);

[[nodiscard]] Matrix3 look_at(Vector3 target, Vector3 up);

}