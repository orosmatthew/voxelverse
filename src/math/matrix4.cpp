#include "matrix4.hpp"

#include "functions.hpp"
#include "matrix3.hpp"
#include "quaternion.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace mve {

Matrix4::Matrix4()
    : col0({ 1.0f, 0.0f, 0.0f, 0.0f })
    , col1({ 0.0f, 1.0f, 0.0f, 0.0f })
    , col2({ 0.0f, 0.0f, 1.0f, 0.0f })
    , col3({ 0.0f, 0.0f, 0.0f, 1.0f })
{
}
Matrix4::Matrix4(const Vector4& col0, const Vector4& col1, const Vector4& col2, const Vector4& col3)
    : col0(col0)
    , col1(col1)
    , col2(col2)
    , col3(col3)
{
}
Matrix4 Matrix4::identity()
{
    return Matrix4(
        { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f });
}
Matrix4 Matrix4::zero()
{
    return Matrix4(
        { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f });
}
float Matrix4::determinant() const
{
    return mve::determinant(*this);
}
float Matrix4::trace() const
{
    return mve::trace(*this);
}
Vector4& Matrix4::operator[](int index)
{
    switch (index) {
    case 0:
        return col0;
    case 1:
        return col1;
    case 2:
        return col2;
    case 3:
        return col3;
    default:
        return col0;
    }
}
Matrix4 Matrix4::transpose() const
{
    return mve::transpose(*this);
}
Matrix4 Matrix4::inverse() const
{
    return mve::inverse(*this);
}
Matrix4 Matrix4::interpolate(const Matrix4& to, float weight) const
{
    return mve::interpolate(*this, to, weight);
}
const Vector4& Matrix4::operator[](int index) const
{
    switch (index) {
    case 0:
        return col0;
    case 1:
        return col1;
    case 2:
        return col2;
    case 3:
        return col3;
    default:
        return col0;
    }
}
Matrix3 Matrix4::basis() const
{
    return mve::basis(*this);
}
Vector3 Matrix4::translation() const
{
    return mve::translation(*this);
}
Quaternion Matrix4::quaternion() const
{
    return Quaternion::from_matrix(basis());
}
Vector3 Matrix4::scale() const
{
    return mve::scale(*this);
}
Matrix4 Matrix4::from_basis_translation(const Matrix3& basis, const Vector3& translation)
{
    Matrix4 result;
    result[0][0] = basis[0][0];
    result[0][1] = basis[0][1];
    result[0][2] = basis[0][2];
    result[1][0] = basis[1][0];
    result[1][1] = basis[1][1];
    result[1][2] = basis[1][2];
    result[2][0] = basis[2][0];
    result[2][1] = basis[2][1];
    result[2][2] = basis[2][2];

    result[3][0] = translation[0];
    result[3][1] = translation[1];
    result[3][2] = translation[2];

    return result;
}
Matrix4 Matrix4::rotate(const Vector3& axis, float angle) const
{
    return mve::rotate(*this, axis, angle);
}
Matrix4 Matrix4::scale(const Vector3& scale) const
{
    return mve::scale(*this, scale);
}
Matrix4 Matrix4::translate(const Vector3& offset) const
{
    return mve::translate(*this, offset);
}
bool Matrix4::is_equal_approx(const Matrix4& matrix) const
{
    return mve::is_equal_approx(*this, matrix);
}
bool Matrix4::is_zero_approx() const
{
    return mve::is_zero_approx(*this);
}
Matrix4 Matrix4::from_rotation_translation(
    const Vector3& rotation_x, const Vector3& rotation_y, const Vector3& rotation_z, const Vector3& translation)
{
    Matrix4 result;
    result[0] = Vector4(rotation_x.x, rotation_x.y, rotation_x.z, 0.0f);
    result[1] = Vector4(rotation_y.x, rotation_y.y, rotation_y.z, 0.0f);
    result[2] = Vector4(rotation_z.x, rotation_z.y, rotation_z.z, 0.0f);
    result[3] = Vector4(translation.x, translation.y, translation.z, 1.0f);
    return result;
}
Matrix4 Matrix4::operator+(const Matrix4& other) const
{
    return Matrix4(col0 + other.col0, col1 + other.col1, col2 + other.col2, col3 + other.col3);
}
void Matrix4::operator+=(const Matrix4& other)
{
    col0 += other.col0;
    col1 += other.col1;
    col2 += other.col2;
    col3 += other.col3;
}
Matrix4 Matrix4::operator-(const Matrix4& other) const
{
    return Matrix4(col0 - other.col0, col1 - other.col1, col2 - other.col2, col3 - other.col3);
}
void Matrix4::operator-=(const Matrix4& other)
{
    col0 -= other.col0;
    col1 -= other.col1;
    col2 -= other.col2;
    col3 -= other.col3;
}
Matrix4 Matrix4::operator*(const Matrix4& other) const
{
    Matrix4 result;

    result[0][0] = (*this)[0][0] * other[0][0] + (*this)[0][1] * other[1][0] + (*this)[0][2] * other[2][0]
        + (*this)[0][3] * other[3][0];
    result[0][1] = (*this)[0][0] * other[0][1] + (*this)[0][1] * other[1][1] + (*this)[0][2] * other[2][1]
        + (*this)[0][3] * other[3][1];
    result[0][2] = (*this)[0][0] * other[0][2] + (*this)[0][1] * other[1][2] + (*this)[0][2] * other[2][2]
        + (*this)[0][3] * other[3][2];
    result[0][3] = (*this)[0][0] * other[0][3] + (*this)[0][1] * other[1][3] + (*this)[0][2] * other[2][3]
        + (*this)[0][3] * other[3][3];
    result[1][0] = (*this)[1][0] * other[0][0] + (*this)[1][1] * other[1][0] + (*this)[1][2] * other[2][0]
        + (*this)[1][3] * other[3][0];
    result[1][1] = (*this)[1][0] * other[0][1] + (*this)[1][1] * other[1][1] + (*this)[1][2] * other[2][1]
        + (*this)[1][3] * other[3][1];
    result[1][2] = (*this)[1][0] * other[0][2] + (*this)[1][1] * other[1][2] + (*this)[1][2] * other[2][2]
        + (*this)[1][3] * other[3][2];
    result[1][3] = (*this)[1][0] * other[0][3] + (*this)[1][1] * other[1][3] + (*this)[1][2] * other[2][3]
        + (*this)[1][3] * other[3][3];
    result[2][0] = (*this)[2][0] * other[0][0] + (*this)[2][1] * other[1][0] + (*this)[2][2] * other[2][0]
        + (*this)[2][3] * other[3][0];
    result[2][1] = (*this)[2][0] * other[0][1] + (*this)[2][1] * other[1][1] + (*this)[2][2] * other[2][1]
        + (*this)[2][3] * other[3][1];
    result[2][2] = (*this)[2][0] * other[0][2] + (*this)[2][1] * other[1][2] + (*this)[2][2] * other[2][2]
        + (*this)[2][3] * other[3][2];
    result[2][3] = (*this)[2][0] * other[0][3] + (*this)[2][1] * other[1][3] + (*this)[2][2] * other[2][3]
        + (*this)[2][3] * other[3][3];
    result[3][0] = (*this)[3][0] * other[0][0] + (*this)[3][1] * other[1][0] + (*this)[3][2] * other[2][0]
        + (*this)[3][3] * other[3][0];
    result[3][1] = (*this)[3][0] * other[0][1] + (*this)[3][1] * other[1][1] + (*this)[3][2] * other[2][1]
        + (*this)[3][3] * other[3][1];
    result[3][2] = (*this)[3][0] * other[0][2] + (*this)[3][1] * other[1][2] + (*this)[3][2] * other[2][2]
        + (*this)[3][3] * other[3][2];
    result[3][3] = (*this)[3][0] * other[0][3] + (*this)[3][1] * other[1][3] + (*this)[3][2] * other[2][3]
        + (*this)[3][3] * other[3][3];

    return result;
}
void Matrix4::operator*=(const Matrix4& other)
{
    *this = *this * other;
}

float determinant(Matrix4 matrix)
{
    float a00 = matrix[0][0], a01 = matrix[1][0], a02 = matrix[2][0], a03 = matrix[3][0];
    float a10 = matrix[0][1], a11 = matrix[1][1], a12 = matrix[2][1], a13 = matrix[3][1];
    float a20 = matrix[0][2], a21 = matrix[1][2], a22 = matrix[2][2], a23 = matrix[3][2];
    float a30 = matrix[0][3], a31 = matrix[1][3], a32 = matrix[2][3], a33 = matrix[3][3];

    return (a30 * a21 * a12 * a03) - (a20 * a31 * a12 * a03) - (a30 * a11 * a22 * a03) + (a10 * a31 * a22 * a03)
        + (a20 * a11 * a32 * a03) - (a10 * a21 * a32 * a03) - (a30 * a21 * a02 * a13) + (a20 * a31 * a02 * a13)
        + (a30 * a01 * a22 * a13) - (a00 * a31 * a22 * a13) - (a20 * a01 * a32 * a13) + (a00 * a21 * a32 * a13)
        + (a30 * a11 * a02 * a23) - (a10 * a31 * a02 * a23) - (a30 * a01 * a12 * a23) + (a00 * a31 * a12 * a23)
        + (a10 * a01 * a32 * a23) - (a00 * a11 * a32 * a23) - (a20 * a11 * a02 * a33) + (a10 * a21 * a02 * a33)
        + (a20 * a01 * a12 * a33) - (a00 * a21 * a12 * a33) - (a10 * a01 * a22 * a33) + (a00 * a11 * a22 * a33);
}
float trace(Matrix4 matrix)
{
    return matrix[0][0] + matrix[1][1] + matrix[2][2] + matrix[3][3];
}
Matrix4 transpose(const Matrix4& matrix)
{
    Matrix4 result;

    result[0][0] = matrix[0][0];
    result[0][1] = matrix[1][0];
    result[0][2] = matrix[2][0];
    result[0][3] = matrix[3][0];
    result[1][0] = matrix[0][1];
    result[1][1] = matrix[1][1];
    result[1][2] = matrix[2][1];
    result[1][3] = matrix[3][1];
    result[2][0] = matrix[0][2];
    result[2][1] = matrix[1][2];
    result[2][2] = matrix[2][2];
    result[2][3] = matrix[3][2];
    result[3][0] = matrix[0][3];
    result[3][1] = matrix[1][3];
    result[3][2] = matrix[2][3];
    result[3][3] = matrix[3][3];

    return result;
}
Matrix4 inverse(Matrix4 matrix)
{
    float a00 = matrix[0][0], a01 = matrix[1][0], a02 = matrix[2][0], a03 = matrix[3][0];
    float a10 = matrix[0][1], a11 = matrix[1][1], a12 = matrix[2][1], a13 = matrix[3][1];
    float a20 = matrix[0][2], a21 = matrix[1][2], a22 = matrix[2][2], a23 = matrix[3][2];
    float a30 = matrix[0][3], a31 = matrix[1][3], a32 = matrix[2][3], a33 = matrix[3][3];

    float c00 = (a00 * a11) - (a01 * a10);
    float c01 = (a00 * a12) - (a02 * a10);
    float c02 = (a00 * a13) - (a03 * a10);
    float c03 = (a01 * a12) - (a02 * a11);
    float c04 = (a01 * a13) - (a03 * a11);
    float c05 = (a02 * a13) - (a03 * a12);
    float c06 = (a20 * a31) - (a21 * a30);
    float c07 = (a20 * a32) - (a22 * a30);
    float c08 = (a20 * a33) - (a23 * a30);
    float c09 = (a21 * a32) - (a22 * a31);
    float c10 = (a21 * a33) - (a23 * a31);
    float c11 = (a22 * a33) - (a23 * a32);

    float inv_det = 1.0f / ((c00 * c11) - (c01 * c10) + (c02 * c09) + (c03 * c08) - (c04 * c07) + (c05 * c06));

    Matrix4 result;

    result[0][0] = ((a11 * c11) - (a12 * c10) + (a13 * c09)) * inv_det;
    result[0][1] = ((-a01 * c11) + (a02 * c10) - (a03 * c09)) * inv_det;
    result[0][2] = ((a31 * c05) - (a32 * c04) + (a33 * c03)) * inv_det;
    result[0][3] = ((-a21 * c05) + (a22 * c04) - (a23 * c03)) * inv_det;
    result[1][0] = ((-a10 * c11) + (a12 * c08) - (a13 * c07)) * inv_det;
    result[1][1] = ((a00 * c11) - (a02 * c08) + (a03 * c07)) * inv_det;
    result[1][2] = ((-a30 * c05) + (a32 * c02) - (a33 * c01)) * inv_det;
    result[1][3] = ((a20 * c05) - (a22 * c02) + (a23 * c01)) * inv_det;
    result[2][0] = ((a10 * c10) - (a11 * c08) + (a13 * c06)) * inv_det;
    result[2][1] = ((-a00 * c10) + (a01 * c08) - (a03 * c06)) * inv_det;
    result[2][2] = ((a30 * c04) - (a31 * c02) + (a33 * c00)) * inv_det;
    result[2][3] = ((-a20 * c04) + (a21 * c02) - (a23 * c00)) * inv_det;
    result[3][0] = ((-a10 * c09) + (a11 * c07) - (a12 * c06)) * inv_det;
    result[3][1] = ((a00 * c09) - (a01 * c07) + (a02 * c06)) * inv_det;
    result[3][2] = ((-a30 * c03) + (a31 * c01) - (a32 * c00)) * inv_det;
    result[3][3] = ((a20 * c03) - (a21 * c01) + (a22 * c00)) * inv_det;

    return result;
}
Matrix3 basis(const Matrix4& matrix)
{
    return Matrix3::from_matrix(matrix);
}
Matrix4 interpolate(Matrix4 from, Matrix4 to, float weight)
{
    Vector3 from_scale = from.scale();
    Quaternion from_rotation = from.quaternion();
    Vector3 from_translation = from.translation();
    Matrix3 from_basis = from.basis();

    Vector3 to_scale = to.scale();
    Quaternion to_rotation = to.quaternion();
    Vector3 to_translation = to.translation();

    Quaternion result_rotation = from_rotation.spherical_linear_interpolate(to_rotation, weight).normalize();
    Vector3 result_scale = from_scale.linear_interpolate(to_scale, weight);
    Matrix3 result_basis = Matrix3::from_quaternion_scale(result_rotation, result_scale);
    Vector3 result_translation = from_translation.linear_interpolate(to_translation, weight);

    return Matrix4::from_basis_translation(result_basis, result_translation);
}
Matrix4 rotate(Matrix4 matrix, Vector3 axis, float angle)
{
    Matrix3 basis = Matrix3::from_axis_angle(axis, angle);
    Vector3 translation = matrix.translation();
    Vector3 new_translation(basis[0].dot(translation), basis[1].dot(translation), basis[2].dot(translation));
    return Matrix4::from_basis_translation(basis * matrix.basis(), new_translation);
}
Vector3 translation(const Matrix4& matrix)
{
    return Vector3(matrix[3][0], matrix[3][1], matrix[3][2]);
}
Vector3 scale(const Matrix4& matrix)
{
    return matrix.basis().scale();
}
Matrix4 perspective(float fov_y, float aspect, float near, float far)
{
    float top = near * tan(fov_y * 0.5f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    float right_left = right - left;
    float top_bottom = top - bottom;
    float far_near = far - near;

    Matrix4 result = Matrix4::zero();

    result[0][0] = (near * 2.0f) / right_left;
    result[1][1] = -(near * 2.0f) / top_bottom;
    result[2][0] = (right + left) / right_left;
    result[2][1] = (top + bottom) / top_bottom;
    result[2][2] = -(far + near) / far_near;
    result[2][3] = -1.0f;
    result[3][2] = -(far * near * 2.0f) / far_near;

    return result;
}
Matrix4 look_at(Vector3 eye, Vector3 target, Vector3 up)
{
    Vector3 target_eye = (target - eye).normalize();
    Vector3 cross_up = target_eye.cross(up).normalize();
    Vector3 cross = cross_up.cross(target_eye);

    Matrix4 result;

    result[0][0] = cross_up.x;
    result[1][0] = cross_up.y;
    result[2][0] = cross_up.z;
    result[0][1] = cross.x;
    result[1][1] = cross.y;
    result[2][1] = cross.z;
    result[0][2] = -target_eye.x;
    result[1][2] = -target_eye.y;
    result[2][2] = -target_eye.z;
    result[3][0] = -dot(cross_up, eye);
    result[3][1] = -dot(cross, eye);
    result[3][2] = dot(target_eye, eye);

    return result;
}
Matrix4 scale(Matrix4 matrix, Vector3 scale)
{
    Matrix3 basis = matrix.basis().scale(scale);
    Vector3 translation = matrix.translation() * scale;
    return Matrix4::from_basis_translation(basis, translation);
}
Matrix4 translate(Matrix4 matrix, Vector3 offset)
{
    Matrix4 result = matrix;
    result[3][0] += offset.x;
    result[3][1] += offset.y;
    result[3][2] += offset.z;
    return result;
}
bool is_equal_approx(Matrix4 a, Matrix4 b)
{
    return mve::is_equal_approx(a[0], b[0]) && mve::is_equal_approx(a[1], b[1]) && mve::is_equal_approx(a[2], b[2])
        && mve::is_equal_approx(a[3], b[3]);
}
bool is_zero_approx(Matrix4 matrix)
{
    return mve::is_zero_approx(matrix[0]) && mve::is_zero_approx(matrix[1]) && mve::is_zero_approx(matrix[2])
        && mve::is_zero_approx(matrix[3]);
}

}