namespace mve {

inline Matrix3::Matrix3()
    : col0(Vector3(1.0f, 0.0f, 0.0f))
    , col1(Vector3(0.0f, 1.0f, 0.0f))
    , col2(Vector3(0.0f, 0.0f, 1.0f))
{
}
inline Matrix3::Matrix3(const Vector3& col0, const Vector3& col1, const Vector3& col2)
    : col0(col0)
    , col1(col1)
    , col2(col2)
{
}

inline Matrix3::Matrix3(
    float c0r0, float c0r1, float c0r2, float c1r0, float c1r1, float c1r2, float c2r0, float c2r1, float c2r2)
    : col0(Vector3(c0r0, c0r1, c0r2))
    , col1(Vector3(c1r0, c1r1, c1r2))
    , col2(Vector3(c2r0, c2r1, c2r2))
{
}
inline Matrix3 Matrix3::zero()
{
    return Matrix3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}
inline Matrix3 Matrix3::identity()
{
    return Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}
inline Vector3 Matrix3::euler() const
{
    return mve::euler(*this);
}
inline Matrix3 Matrix3::from_axis_angle(const Vector3& axis, float angle)
{
    Matrix3 result;

    Vector3 axis_norm = axis.normalize();

    float sin_angle = sin(angle);
    float cos_angle = cos(angle);
    float t = 1.0f - cos_angle;

    result[0][0] = axis_norm.x * axis_norm.x * t + cos_angle;
    result[0][1] = axis_norm.y * axis_norm.x * t + axis_norm.z * sin_angle;
    result[0][2] = axis_norm.z * axis_norm.x * t - axis_norm.y * sin_angle;

    result[1][0] = axis_norm.x * axis_norm.y * t - axis_norm.z * sin_angle;
    result[1][1] = axis_norm.y * axis_norm.y * t + cos_angle;
    result[1][2] = axis_norm.z * axis_norm.y * t + axis_norm.x * sin_angle;

    result[2][0] = axis_norm.x * axis_norm.z * t + axis_norm.y * sin_angle;
    result[2][1] = axis_norm.y * axis_norm.z * t - axis_norm.x * sin_angle;
    result[2][2] = axis_norm.z * axis_norm.z * t + cos_angle;

    return result;
}
inline Matrix3 Matrix3::from_euler(const Vector3& euler)
{
    float c = cos(euler.x);
    float s = sin(euler.x);
    Matrix3 x { 1.0f, 0.0f, 0.0f, 0.0f, c, -s, 0.0f, s, c };

    c = cos(euler.y);
    s = sin(euler.y);
    Matrix3 y { c, 0.0f, s, 0.0f, 1.0f, 0.0f, -s, 0.0f, c };

    c = cos(euler.z);
    s = sin(euler.z);
    Matrix3 z { c, -s, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 1.0f };

    return x * (y * z);
}
inline Vector3 Matrix3::scale() const
{
    return mve::scale(*this);
}
inline float Matrix3::determinant() const
{
    return mve::determinant(*this);
}
inline float Matrix3::trace() const
{
    return mve::trace(*this);
}
inline Matrix3 Matrix3::transpose() const
{
    return mve::transpose(*this);
}
inline Matrix3 Matrix3::inverse() const
{
    return mve::inverse(*this);
}
inline Matrix3 Matrix3::orthonormalize() const
{
    return mve::orthonormalize(*this);
}
inline Matrix3 Matrix3::from_quaternion(const Quaternion& quaternion)
{
    Matrix3 result;
    result[0][0] = 2 * (quaternion.x * quaternion.x + quaternion.y * quaternion.y) - 1;
    result[0][1] = 2 * (quaternion.y * quaternion.z - quaternion.x * quaternion.w);
    result[0][2] = 2 * (quaternion.y * quaternion.w + quaternion.x * quaternion.z);

    result[1][0] = 2 * (quaternion.y * quaternion.z + quaternion.x * quaternion.w);
    result[1][1] = 2 * (quaternion.x * quaternion.x + quaternion.z * quaternion.z) - 1;
    result[1][2] = 2 * (quaternion.z * quaternion.w - quaternion.x * quaternion.y);

    result[2][0] = 2 * (quaternion.y * quaternion.w - quaternion.x * quaternion.z);
    result[2][1] = 2 * (quaternion.z * quaternion.w + quaternion.x * quaternion.y);
    result[2][2] = 2 * (quaternion.x * quaternion.x + quaternion.w * quaternion.w) - 1;

    return result;
}
inline Matrix3 Matrix3::spherical_linear_interpolate(const Matrix3& to, float weight) const
{
    return mve::spherical_linear_interpolate(*this, to, weight);
}
inline Matrix3 Matrix3::rotate(const Vector3& axis, float angle) const
{
    return mve::rotate(*this, axis, angle);
}
inline Matrix3 Matrix3::scale(const Vector3& scale) const
{
    return mve::scale(*this, scale);
}
inline bool Matrix3::is_equal_approx(const Matrix3& matrix) const
{
    return mve::is_equal_approx(*this, matrix);
}
inline bool Matrix3::is_zero_approx() const
{
    return mve::is_zero_approx(*this);
}
inline Vector3& Matrix3::operator[](int index)
{
    switch (index) {
    case 0:
        return col0;
    case 1:
        return col1;
    case 2:
        return col2;
    default:
        return col0;
    }
}
inline const Vector3& Matrix3::operator[](int index) const
{
    switch (index) {
    case 0:
        return col0;
    case 1:
        return col1;
    case 2:
        return col2;
    default:
        return col0;
    }
}
inline Matrix3 Matrix3::operator+(const Matrix3& other) const
{
    auto result = *this;
    result[0] += other[0];
    result[1] += other[1];
    result[2] += other[2];
    return result;
}
inline void Matrix3::operator+=(const Matrix3& other)
{
    col0 += other[0];
    col1 += other[1];
    col2 += other[2];
}
inline Matrix3 Matrix3::operator-(const Matrix3& other) const
{
    auto result = *this;
    result[0] -= other[0];
    result[1] -= other[1];
    result[2] -= other[2];
    return result;
}
inline void Matrix3::operator-=(const Matrix3& other)
{
    col0 -= other[0];
    col1 -= other[1];
    col2 -= other[2];
}

inline Matrix3 Matrix3::operator*(const Matrix3& other) const
{
    Matrix3 result;
    result[0][0] = (*this)[0][0] * other[0][0] + (*this)[0][1] * other[1][0] + (*this)[0][2] * other[2][0];
    result[0][1] = (*this)[0][0] * other[0][1] + (*this)[0][1] * other[1][1] + (*this)[0][2] * other[2][1];
    result[0][2] = (*this)[0][0] * other[0][2] + (*this)[0][1] * other[1][2] + (*this)[0][2] * other[2][2];
    result[1][0] = (*this)[1][0] * other[0][0] + (*this)[1][1] * other[1][0] + (*this)[1][2] * other[2][0];
    result[1][1] = (*this)[1][0] * other[0][1] + (*this)[1][1] * other[1][1] + (*this)[1][2] * other[2][1];
    result[1][2] = (*this)[1][0] * other[0][2] + (*this)[1][1] * other[1][2] + (*this)[1][2] * other[2][2];
    result[2][0] = (*this)[2][0] * other[0][0] + (*this)[2][1] * other[1][0] + (*this)[2][2] * other[2][0];
    result[2][1] = (*this)[2][0] * other[0][1] + (*this)[2][1] * other[1][1] + (*this)[2][2] * other[2][1];
    result[2][2] = (*this)[2][0] * other[0][2] + (*this)[2][1] * other[1][2] + (*this)[2][2] * other[2][2];
    return result;
}
inline void Matrix3::operator*=(const Matrix3& other)
{
    *this = *this * other;
}
inline Matrix3 Matrix3::operator*(float val) const
{
    Matrix3 result = *this;
    result[0] *= val;
    result[1] *= val;
    result[2] *= val;
    return result;
}
inline void Matrix3::operator*=(float val)
{
    col0 *= val;
    col1 *= val;
    col2 *= val;
}
inline Matrix3 Matrix3::operator*(int val) const
{
    Matrix3 result = *this;
    result[0] *= val;
    result[1] *= val;
    result[2] *= val;
    return result;
}
inline void Matrix3::operator*=(int val)
{
    col0 *= val;
    col1 *= val;
    col2 *= val;
}
inline bool Matrix3::operator!=(const Matrix3& other) const
{
    if (col0 != other.col0) {
        return false;
    }
    if (col1 != other.col1) {
        return false;
    }
    if (col2 != other.col2) {
        return false;
    }
    return true;
}
inline bool Matrix3::operator==(const Matrix3& other) const
{
    return col0 == other.col0 && col1 == other.col1 && col2 == other.col2;
}
inline bool Matrix3::operator<(const Matrix3& other) const
{
    if (col0 != other.col0) {
        return col0 < other.col0;
    }
    if (col1 != other.col1) {
        return col1 < other.col1;
    }
    if (col2 != other.col2) {
        return col2 < other.col2;
    }
    return false;
}
inline bool Matrix3::operator<=(const Matrix3& other) const
{
    if (col0 != other.col0) {
        return col0 < other.col0;
    }
    if (col1 != other.col1) {
        return col1 < other.col1;
    }
    if (col2 != other.col2) {
        return col2 < other.col2;
    }
    return true;
}
inline bool Matrix3::operator>(const Matrix3& other) const
{
    if (col0 != other.col0) {
        return col0 > other.col0;
    }
    if (col1 != other.col1) {
        return col1 > other.col1;
    }
    if (col2 != other.col2) {
        return col2 > other.col2;
    }
    return false;
}
inline bool Matrix3::operator>=(const Matrix3& other) const
{
    if (col0 != other.col0) {
        return col0 > other.col0;
    }
    if (col1 != other.col1) {
        return col1 > other.col1;
    }
    if (col2 != other.col2) {
        return col2 > other.col2;
    }
    return true;
}
inline Matrix3 Matrix3::look_at(const Vector3& target, const Vector3& up)
{
    return mve::look_at(target, up);
}
inline Matrix3 Matrix3::from_matrix(const Matrix4& matrix)
{
    Matrix3 result;
    result[0] = Vector3(matrix[0][0], matrix[0][1], matrix[0][2]);
    result[1] = Vector3(matrix[1][0], matrix[1][1], matrix[1][2]);
    result[2] = Vector3(matrix[2][0], matrix[2][1], matrix[2][2]);
    return result;
}
inline Quaternion Matrix3::quaternion() const
{
    return Quaternion::from_matrix(*this);
}
inline Matrix3 Matrix3::from_quaternion_scale(const Quaternion& quaternion, const Vector3& scale)
{
    Matrix3 result;

    result[0][0] = scale.x;
    result[1][1] = scale.y;
    result[2][2] = scale.z;

    result = result.rotate(quaternion);
    return result;
}
inline Matrix3 Matrix3::rotate(const Quaternion& quaternion) const
{
    return mve::rotate(*this, quaternion);
}
inline Matrix4 Matrix3::with_translation(const Vector3 translation) const
{
    return mve::with_translation(*this, translation);
}
inline Matrix3 Matrix3::from_scale(const Vector3& scale)
{
    return Matrix3::identity().scale(scale);
}
inline Vector3 Matrix3::operator*(Vector3 vector) const
{
    const Matrix3& m = *this;
    return { m[0][0] * vector.x + m[0][1] * vector.y + m[0][2] * vector.z,
             m[1][0] * vector.x + m[1][1] * vector.y + m[1][2] * vector.z,
             m[2][0] * vector.x + m[2][1] * vector.y + m[2][2] * vector.z };
}
inline Matrix3 Matrix3::from_direction(const Vector3& dir, const Vector3& up)
{
    Vector3 axis_x = up.cross(dir).normalize();
    Vector3 axis_y = dir.cross(axis_x).normalize();

    Matrix3 result;

    result[0][0] = axis_x.x;
    result[0][1] = axis_y.x;
    result[0][2] = dir.x;
    result[1][0] = axis_x.y;
    result[1][1] = axis_y.y;
    result[1][2] = dir.y;
    result[2][0] = axis_x.z;
    result[2][1] = axis_y.z;
    result[2][2] = dir.z;

    return result;
}

inline Vector3 euler(const Matrix3& matrix)
{
    Vector3 euler;
    float sy = matrix[2][0];
    if (sy < 1.0f) {
        if (sy > -1.0f) {
            if (matrix[0][1] == 0.0f && matrix[1][0] == 0.0f && matrix[2][1] == 0.0f && matrix[1][2] == 0.0f
                && matrix[1][1] == 1.0f) {
                euler.x = 0.0f;
                euler.y = atan2(matrix[2][0], matrix[0][0]);
                euler.z = 0.0f;
            }
            else {
                euler.x = atan2(-matrix[2][1], matrix[2][2]);
                euler.y = asin(sy);
                euler.z = atan2(-matrix[1][0], matrix[0][0]);
            }
        }
        else {
            euler.x = atan2(matrix[1][2], matrix[1][1]);
            euler.y = -pi / 2.0f;
            euler.z = 0.0f;
        }
    }
    else {
        euler.x = atan2(matrix[1][2], matrix[1][1]);
        euler.y = pi / 2.0f;
        euler.z = 0.0f;
    }
    return euler;
}
inline Vector3 scale(const Matrix3& matrix)
{
    Vector3 scale_abs = Vector3(
        Vector3(matrix[0][0], matrix[0][1], matrix[0][2]).length(),
        Vector3(matrix[1][0], matrix[1][1], matrix[1][2]).length(),
        Vector3(matrix[2][0], matrix[2][1], matrix[2][2]).length());

    if (matrix.determinant() > 0) {
        return scale_abs;
    }
    else {
        return -scale_abs;
    }
}
inline float determinant(const Matrix3& matrix)
{
    float a00 = matrix[0][0], a01 = matrix[1][0], a02 = matrix[2][0];
    float a10 = matrix[0][1], a11 = matrix[1][1], a12 = matrix[2][1];
    float a20 = matrix[0][2], a21 = matrix[1][2], a22 = matrix[2][2];

    return (a00 * a11 * a22) + (a01 * a12 * a20) + (a02 * a10 * a21) - (a02 * a11 * a20) - (a01 * a10 * a22)
        - (a00 * a12 * a21);
}
inline float trace(const Matrix3& matrix)
{
    return matrix[0][0] + matrix[1][1] + matrix[2][2];
}
inline Matrix3 transpose(const Matrix3& matrix)
{
    Matrix3 result;

    result[0][0] = matrix[0][0];
    result[0][1] = matrix[1][0];
    result[0][2] = matrix[2][0];
    result[1][0] = matrix[0][1];
    result[1][1] = matrix[1][1];
    result[1][2] = matrix[2][1];
    result[2][0] = matrix[0][2];
    result[2][1] = matrix[1][2];
    result[2][2] = matrix[2][2];

    return result;
}
inline Matrix3 inverse(const Matrix3& matrix)
{
    float a11 = matrix[0][0], a12 = matrix[1][0], a13 = matrix[2][0];
    float a21 = matrix[0][1], a22 = matrix[1][1], a23 = matrix[2][1];
    float a31 = matrix[0][2], a32 = matrix[1][2], a33 = matrix[2][2];

    float inv_det = 1.0f
        / ((a11 * a22 * a33) + (a12 * a33 * a31) + (a13 * a21 * a32) - (a13 * a22 * a31) - (a12 * a21 * a33)
           - (a11 * a23 * a32));

    Matrix3 result;

    result[0][0] = ((a22 * a33) - (a23 * a32)) * inv_det;
    result[0][1] = -((a21 * a33) - (a23 * a31)) * inv_det;
    result[0][2] = ((a21 * a32) - (a22 * a31)) * inv_det;
    result[1][0] = -((a12 * a33) - (a13 * a32)) * inv_det;
    result[1][1] = ((a11 * a33) - (a13 * a31)) * inv_det;
    result[1][2] = -((a11 * a32) - (a12 * a31)) * inv_det;
    result[2][0] = ((a12 * a23) - (a13 * a22)) * inv_det;
    result[2][1] = -((a11 * a23) - (a13 * a21)) * inv_det;
    result[2][2] = ((a11 * a22) - (a12 * a21)) * inv_det;

    return result;
}
inline Matrix3 spherical_linear_interpolate(const Matrix3& from, const Matrix3& to, float weight)
{
    Quaternion from_quat = from.quaternion();
    Quaternion to_quat = to.quaternion();

    Matrix3 matrix = from_quat.spherical_linear_interpolate(to_quat, weight).matrix();
    matrix[0] *= linear_interpolate(from[0].length(), to[0].length(), weight);
    matrix[1] *= linear_interpolate(from[1].length(), to[1].length(), weight);
    matrix[2] *= linear_interpolate(from[2].length(), to[2].length(), weight);

    return matrix;
}
inline Matrix3 orthonormalize(const Matrix3& matrix)
{
    Vector3 x = matrix.col0;
    Vector3 y = matrix.col1;
    Vector3 z = matrix.col2;

    x = mve::normalize(x);
    y = (y - x * (mve::dot(x, y)));
    y = mve::normalize(y);
    z = (z - x * (mve::dot(x, z)) - y * (mve::dot(y, z)));
    z = mve::normalize(z);

    return Matrix3(x, y, z);
}
inline Matrix3 rotate(const Matrix3& matrix, const Vector3& axis, float angle)
{
    return Matrix3::from_axis_angle(axis, angle) * matrix;
}
inline Matrix3 scale(const Matrix3& matrix, const Vector3& scale)
{
    Matrix3 result = matrix;
    result[0][0] *= scale.x;
    result[1][0] *= scale.x;
    result[2][0] *= scale.x;
    result[0][1] *= scale.y;
    result[1][1] *= scale.y;
    result[2][1] *= scale.y;
    result[0][2] *= scale.z;
    result[1][2] *= scale.z;
    result[2][2] *= scale.z;

    return result;
}
inline bool is_equal_approx(const Matrix3& a, const Matrix3& b)
{
    for (int c = 0; c < 3; c++) {
        if (!is_equal_approx(a[c], b[c])) {
            return false;
        }
    }
    return true;
}
inline bool is_zero_approx(const Matrix3& matrix)
{
    for (int c = 0; c < 3; c++) {
        if (!is_zero_approx(matrix[c])) {
            return false;
        }
    }
    return true;
}
inline Matrix3 look_at(const Vector3& target, const Vector3& up)
{
    Matrix3 result;

    Vector3 vz = mve::normalize(-target);
    Vector3 vx = mve::normalize(up.cross(vz));
    Vector3 vy = vz.cross(vx);

    result[0][0] = vx.x;
    result[0][1] = vy.x;
    result[0][2] = vz.x;
    result[1][0] = vx.y;
    result[1][1] = vy.y;
    result[1][2] = vz.y;
    result[2][0] = vx.z;
    result[2][1] = vy.z;
    result[2][2] = vz.z;

    return result;
}
inline Matrix3 rotate(const Matrix3& matrix, const Quaternion& quaternion)
{
    return quaternion.matrix() * matrix;
}
inline Matrix4 with_translation(const Matrix3& matrix, const Vector3 translation)
{
    return Matrix4::from_basis_translation(matrix, translation);
}
}