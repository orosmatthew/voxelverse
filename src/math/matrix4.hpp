#pragma once

#include "vector4.hpp"

namespace mve {

class Vector3;
class Vector4;
class Matrix3;
class Quaternion;

class Matrix4 {
public:
    Vector4 col0;
    Vector4 col1;
    Vector4 col2;
    Vector4 col3;

    Matrix4();

    Matrix4(const Vector4& col0, const Vector4& col1, const Vector4& col2, const Vector4& col3);

    [[nodiscard]] static Matrix4 zero();

    [[nodiscard]] static Matrix4 identity();

    [[nodiscard]] static Matrix4 from_rotation_translation(
        const Vector3& rotation_x, const Vector3& rotation_y, const Vector3& rotation_z, const Vector3& translation);

    [[nodiscard]] static Matrix4 from_basis_translation(const Matrix3& basis, const Vector3& translation);

    [[nodiscard]] float determinant() const;

    [[nodiscard]] float trace() const;

    [[nodiscard]] Matrix4 transpose() const;

    [[nodiscard]] Matrix4 inverse() const;

    [[nodiscard]] Matrix4 interpolate(const Matrix4& to, float weight) const;

    [[nodiscard]] Matrix4 rotate(const Vector3& axis, float angle) const;

    [[nodiscard]] Matrix4 scale(const Vector3& scale) const;

    [[nodiscard]] Matrix4 translate(const Vector3& offset) const;

    [[nodiscard]] bool is_equal_approx(const Matrix4& matrix) const;

    [[nodiscard]] bool is_zero_approx() const;

    [[nodiscard]] Matrix3 basis() const;

    [[nodiscard]] Vector3 translation() const;

    [[nodiscard]] Quaternion quaternion() const;

    [[nodiscard]] Vector3 scale() const;

    Vector4& operator[](int index);

    const Vector4& operator[](int index) const;

    [[nodiscard]] Matrix4 operator+(const Matrix4& other) const;

    void operator+=(const Matrix4& other);

    [[nodiscard]] Matrix4 operator-(const Matrix4& other) const;

    void operator-=(const Matrix4& other);

    [[nodiscard]] Matrix4 operator*(const Matrix4& other) const;

    void operator*=(const Matrix4& other);
};

[[nodiscard]] float determinant(Matrix4 matrix);

[[nodiscard]] float trace(Matrix4 matrix);

[[nodiscard]] Matrix4 transpose(const Matrix4& matrix);

[[nodiscard]] Matrix4 inverse(Matrix4 matrix);

[[nodiscard]] Matrix4 interpolate(Matrix4 from, Matrix4 to, float weight);

[[nodiscard]] Matrix4 rotate(Matrix4 matrix, Vector3 axis, float angle);

[[nodiscard]] Matrix4 scale(Matrix4 matrix, Vector3 scale);

[[nodiscard]] Matrix4 translate(Matrix4 matrix, Vector3 offset);

[[nodiscard]] bool is_equal_approx(Matrix4 a, Matrix4 b);

[[nodiscard]] bool is_zero_approx(Matrix4 matrix);

[[nodiscard]] Matrix3 basis(const Matrix4& matrix);

[[nodiscard]] Matrix4 frustum(float left, float right, float bottom, float top, float near, float far);

[[nodiscard]] Matrix4 perspective(float fov_y, float aspect, float near, float far);

[[nodiscard]] Matrix4 ortho(float left, float right, float bottom, float top, float near, float far);

[[nodiscard]] Matrix4 look_at(Vector3 eye, Vector3 target, Vector3 up);

[[nodiscard]] Vector3 translation(const Matrix4& matrix);

[[nodiscard]] Vector3 scale(const Matrix4& matrix);

}