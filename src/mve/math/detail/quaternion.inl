namespace mve {

inline Quaternion::Quaternion()
    : data(0.0f, 0.0f, 0.0f, 0.0f)
{
}
inline Quaternion::Quaternion(const Vector4& vector)
    : data(vector)
{
}
inline Quaternion::Quaternion(float x, float y, float z, float w)
    : data(x, y, z, w)
{
}
inline Quaternion Quaternion::from_axis_angle(const Vector3& axis, float angle)
{
    Vector4 data;

    float axis_length = mve::length(axis);
    if (axis_length == 0) {
        data.x = 0.0f;
        data.y = 0.0f;
        data.z = 0.0f;
        data.w = 0.0f;
    }
    else {
        float angle_sin = sin(angle * 0.5f);
        float angle_cos = cos(angle * 0.5f);
        float s = angle_sin / axis_length;
        data.x = axis.x * s;
        data.y = axis.y * s;
        data.z = axis.z * s;
        data.w = angle_cos;
    }

    return Quaternion(data);
}
inline Quaternion Quaternion::from_euler(const Vector3& euler)
{
    Vector3 half = euler / 2.0f;

    float cos_x = cos(half.x);
    float sin_x = sin(half.x);
    float cos_y = cos(half.y);
    float sin_y = sin(half.y);
    float cos_z = cos(half.z);
    float sin_z = sin(half.z);

    return Quaternion(
        sin_x * cos_y * sin_z + cos_x * sin_y * cos_z,
        sin_x * cos_y * cos_z - cos_x * sin_y * sin_z,
        -sin_x * sin_y * cos_z + cos_x * cos_y * sin_z,
        sin_x * sin_y * sin_z + cos_x * cos_y * cos_z);
}
inline float Quaternion::angle_to(const Quaternion& to) const
{
    return mve::angle(*this, to);
}
inline float Quaternion::dot(const Quaternion& quaternion) const
{
    return mve::dot(*this, quaternion);
}
inline Vector3 Quaternion::euler() const
{
    return mve::euler(*this);
}
inline Quaternion Quaternion::inverse() const
{
    return mve::inverse(*this);
}
inline bool Quaternion::is_equal_approx(const Quaternion& quaternion) const
{
    return mve::is_equal_approx(*this, quaternion);
}
inline bool Quaternion::is_zero_approx() const
{
    return mve::is_zero_approx(*this);
}
inline float Quaternion::length() const
{
    return mve::length(*this);
}
inline float Quaternion::length_sqrd() const
{
    return mve::length_sqrd(*this);
}
inline Quaternion Quaternion::spherical_linear_interpolate(const Quaternion& to, float weight) const
{
    return mve::spherical_linear_interpolate(*this, to, weight);
}
inline bool Quaternion::operator!=(const Quaternion& other) const
{
    return data != other.data;
}
inline Quaternion Quaternion::operator*(const Quaternion& other) const
{
    return Quaternion(
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y + y * other.w + z * other.x - x * other.z,
        w * other.z + z * other.w + x * other.y - y * other.x,
        w * other.w - x * other.x - y * other.y - z * other.z);
}
inline void Quaternion::operator*=(const Quaternion& other)
{
    float new_x = w * other.x + x * other.w + y * other.z - z * other.y;
    float new_y = w * other.y + y * other.w + z * other.x - x * other.z;
    float new_z = w * other.z + z * other.w + x * other.y - y * other.x;
    w = w * other.w - x * other.x - y * other.y - z * other.z;
    x = new_x;
    y = new_y;
    z = new_z;
}
inline Quaternion Quaternion::operator*(float value) const
{
    return data * value;
}
inline void Quaternion::operator*=(float value)
{
    data *= value;
}
inline Quaternion Quaternion::operator*(int value) const
{
    return data * value;
}
inline void Quaternion::operator*=(int value)
{
    data *= value;
}
inline Quaternion Quaternion::operator+(const Quaternion& other) const
{
    return data + other.data;
}
inline void Quaternion::operator+=(const Quaternion& other)
{
    data += other.data;
}
inline Quaternion Quaternion::operator-(const Quaternion& other) const
{
    return data - other.data;
}
inline void Quaternion::operator-=(const Quaternion& other)
{
    data -= other.data;
}
inline Quaternion Quaternion::operator/(float value) const
{
    return data * (1.0f / value);
}
inline void Quaternion::operator/=(float value)
{
    data *= 1.0f / value;
}
inline Quaternion Quaternion::operator/(int value) const
{
    return data * (1.0f / value);
}
inline void Quaternion::operator/=(int value)
{
    data *= 1.0f / value;
}
inline bool Quaternion::operator==(const Quaternion& other) const
{
    return data == other.data;
}
inline float& Quaternion::operator[](int index)
{
    return data[index];
}
inline Quaternion Quaternion::operator+() const
{
    return Quaternion(data);
}
inline Quaternion Quaternion::operator-() const
{
    return -data;
}
inline Quaternion Quaternion::from_matrix(const Matrix3& matrix)
{
    float trace = matrix.trace();
    Quaternion result;

    if (trace > 0.0f) {
        float s = sqrt(trace + 1.0f);
        result.w = s * 0.5f;
        s = 0.5f / s;

        result.x = ((matrix[1][2] - matrix[2][1]) * s);
        result.y = ((matrix[2][0] - matrix[0][2]) * s);
        result.z = ((matrix[0][1] - matrix[1][0]) * s);
    }
    else {
        int i;
        if (matrix[0][0] < matrix[1][1]) {
            i = matrix[1][1] < matrix[2][2] ? 2 : 1;
        }
        else {
            i = matrix[0][0] < matrix[2][2] ? 2 : 0;
        }
        int j = (i + 1) % 3;
        int k = (i + 2) % 3;

        float s = sqrt(matrix[i][i] - matrix[j][j] - matrix[k][k] + 1.0f);
        result[i] = s * 0.5f;
        s = 0.5f / s;

        result.w = (matrix[j][k] - matrix[k][j]) * s;
        result[j] = (matrix[i][j] + matrix[j][i]) * s;
        result[k] = (matrix[i][k] + matrix[k][i]) * s;
    }

    return result;
}
inline Matrix3 Quaternion::matrix() const
{
    return Matrix3::from_quaternion(*this);
}
inline Quaternion Quaternion::normalize() const
{
    return mve::normalize(*this);
}
inline const float& Quaternion::operator[](int index) const
{
    return data[index];
}
inline Quaternion Quaternion::from_direction(const Vector3& dir, const Vector3& up)
{
    Quaternion result;
    float angle = mve::atan2(dir.x, dir.z);
    result.x = up.x;
    result.y = up.y * mve::sin(angle / 2.0f);
    result.z = up.z;
    result.w = mve::cos(angle / 2.0f);
    return result;
}
inline Quaternion Quaternion::from_vector3_to_vector3(const Vector3& from, const Vector3& to)
{
    Quaternion result;

    float cos2Theta = (from.x*to.x + from.y*to.y + from.z*to.z);    // Vector3DotProduct(from, to)
    Vector3 cross = { from.y*to.z - from.z*to.y, from.z*to.x - from.x*to.z, from.x*to.y - from.y*to.x }; // Vector3CrossProduct(from, to)

    result.x = cross.x;
    result.y = cross.y;
    result.z = cross.z;
    result.w = 1.0f + cos2Theta;

    // QuaternionNormalize(q);
    // NOTE: Normalize to essentially nlerp the original and identity to 0.5
    Quaternion q = result;
    float length = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (length == 0.0f) length = 1.0f;
    float ilength = 1.0f/length;

    result.x = q.x*ilength;
    result.y = q.y*ilength;
    result.z = q.z*ilength;
    result.w = q.w*ilength;

    return result;
}

inline float angle(const Quaternion& from, const Quaternion& to)
{
    float dot = mve::dot(from, to);
    return acos(clamp(sqrd(dot) * 2.0f - 1.0f, -1.0f, 1.0f));
}
inline float dot(const Quaternion& a, const Quaternion& b)
{
    return mve::dot(a.data, b.data);
}
inline Vector3 euler(const Quaternion& quaternion)
{
    return quaternion.matrix().euler();
}
inline Quaternion inverse(const Quaternion& quaternion)
{
    return Quaternion(-quaternion.x, -quaternion.y, -quaternion.z, quaternion.w);
}
inline bool is_equal_approx(const Quaternion& a, const Quaternion& b)
{
    return is_equal_approx(a.x, b.x) && is_equal_approx(a.y, b.y) && is_equal_approx(a.z, b.z)
        && is_equal_approx(a.w, b.w);
}
inline bool is_zero_approx(const Quaternion& quaternion)
{
    return is_zero_approx(quaternion.x) && is_zero_approx(quaternion.y) && is_zero_approx(quaternion.z)
        && is_zero_approx(quaternion.w);
}
inline float length(const Quaternion& quaternion)
{
    return quaternion.data.length();
}
inline float length_sqrd(const Quaternion& quaternion)
{
    return quaternion.data.length_sqrd();
}
inline Quaternion spherical_linear_interpolate(const Quaternion& from, const Quaternion& to, float weight)
{
    float dot = mve::dot(from, to);

    Quaternion to_new;
    if (dot < 0.0f) {
        dot = -dot;
        to_new = -to;
    }
    else {
        to_new = to;
    }

    float scale0;
    float scale1;
    if ((1.0f - dot) > epsilon) {
        float omega = acos(dot);
        float omega_sin = sin(omega);
        scale0 = sin((1.0f - weight) * omega) / omega_sin;
        scale1 = sin(weight * omega) / omega_sin;
    }
    else {
        scale0 = 1.0f - weight;
        scale1 = weight;
    }

    return Quaternion(
        scale0 * from.x + scale1 * to_new.x,
        scale0 * from.y + scale1 * to_new.y,
        scale0 * from.z + scale1 * to_new.z,
        scale0 * from.w + scale1 * to_new.w);
}
inline Matrix3 matrix(const Quaternion& quaternion)
{
    return Matrix3::from_quaternion(quaternion);
}
inline Quaternion normalize(const Quaternion& quaternion)
{
    return quaternion / quaternion.length();
}

}