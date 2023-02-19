namespace mve {

inline Vector3 abs(Vector3 vector)
{
    return Vector3(abs(vector.x), abs(vector.y), abs(vector.z));
}
inline Vector3 ceil(Vector3 vector)
{
    return Vector3(ceil(vector.x), ceil(vector.y), ceil(vector.z));
}
inline Vector3 clamp(Vector3 vector, Vector3 min, Vector3 max)
{
    return Vector3(clamp(vector.x, min.x, max.x), clamp(vector.y, min.y, max.y), clamp(vector.z, min.z, max.z));
}
inline Vector3 direction(Vector3 from, Vector3 to)
{
    return normalize(to - from);
}
inline float distance_sqrd(Vector3 from, Vector3 to)
{
    float diff_x = to.x - from.x;
    float diff_y = to.y - from.y;
    float diff_z = to.z - from.z;
    return sqrd(diff_x) + sqrd(diff_y) + sqrd(diff_z);
}
inline float distance(Vector3 from, Vector3 to)
{
    float diff_x = to.x - from.x;
    float diff_y = to.y - from.y;
    float diff_z = to.z - from.z;
    return sqrt(sqrd(diff_x) + sqrd(diff_y) + sqrd(diff_z));
}
inline Vector3 normalize(Vector3 vector)
{
    Vector3 result(0.0f);
    float length = mve::length(vector);

    if (length > 0) {
        float inverse_length = 1.0f / length;
        result = vector * inverse_length;
    }

    return result;
}
inline Vector3 floor(Vector3 vector)
{
    return Vector3(floor(vector.x), floor(vector.y), floor(vector.z));
}
inline float length(Vector3 vector)
{
    return sqrt(sqrd(vector.x) + sqrd(vector.y) + sqrd(vector.z));
}
inline float length_sqrd(Vector3 vector)
{
    return sqrd(vector.x) + sqrd(vector.y) + sqrd(vector.z);
}
inline Vector3 linear_interpolate(Vector3 from, Vector3 to, float weight)
{
    return Vector3(
        linear_interpolate(from.x, to.x, weight),
        linear_interpolate(from.y, to.y, weight),
        linear_interpolate(from.z, to.z, weight));
}
inline Vector3Axis max_axis(Vector3 vector)
{
    float max_val = vector.x;
    Vector3Axis max_axis = Vector3Axis::x;

    if (vector.y > max_val) {
        max_val = vector.y;
        max_axis = Vector3Axis::y;
    }

    if (vector.z > max_val) {
        max_val = vector.z;
        max_axis = Vector3Axis::z;
    }

    return max_axis;
}
inline Vector3Axis min_axis(Vector3 vector)
{
    float min_val = vector.x;
    Vector3Axis min_axis = Vector3Axis::x;

    if (vector.y < min_val) {
        min_val = vector.y;
        min_axis = Vector3Axis::y;
    }

    if (vector.z < min_val) {
        min_val = vector.z;
        min_axis = Vector3Axis::z;
    }

    return min_axis;
}
inline bool is_equal_approx(Vector3 a, Vector3 b)
{
    return is_equal_approx(a.x, b.x) && is_equal_approx(a.y, b.y) && is_equal_approx(a.z, b.z);
}
inline bool is_zero_approx(Vector3 vector)
{
    return is_zero_approx(vector.x) && is_zero_approx(vector.y) && is_zero_approx(vector.z);
}
inline Vector3 move_toward(Vector3 from, Vector3 to, float amount)
{
    if (from.distance_to(to) >= amount) {
        return to;
    }
    Vector3 result = from;
    result += from.direction_to(to) * amount;
    return result;
}
inline float dot(Vector3 a, Vector3 b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}
inline Vector3 cross(Vector3 a, Vector3 b)
{
    return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
inline Vector3 reflect(Vector3 vector, Vector3 normal)
{
    Vector3 result;

    float dot = (vector.x * normal.x + vector.y * normal.y + vector.z * normal.z);

    result.x = vector.x - (2.0f * normal.x) * dot;
    result.y = vector.y - (2.0f * normal.y) * dot;
    result.z = vector.z - (2.0f * normal.z) * dot;

    return result;
}
inline Vector3 inverse(Vector3 vector)
{
    return Vector3(1.0f / vector.x, 1.0f / vector.y, 1.0f / vector.z);
}
inline Vector3 clamp_length(Vector3 vector, float min, float max)
{
    float length_sqr = mve::length_sqrd(vector);
    if (length_sqr > 0.0f) {
        float length = sqrt(length_sqr);
        if (length < min) {
            return normalize(vector) * min;
        }
        else if (length > max) {
            return normalize(vector) * max;
        }
    }
    return vector;
}
inline Vector3 round(Vector3 vector)
{
    return Vector3(round(vector.x), round(vector.y), round(vector.z));
}
float angle(Vector3 a, Vector3 b)
{
    return atan2(mve::length(mve::cross(a, b)), mve::dot(a, b));
}
inline Vector3 rotate(Vector3 vector, Vector3 axis, float angle)
{
    axis = mve::normalize(axis);

    angle /= 2.0f;
    float a = sin(angle);
    float b = axis.x * a;
    float c = axis.y * a;
    float d = axis.z * a;
    a = cos(angle);
    Vector3 w(b, c, d);

    Vector3 wv = mve::cross(w, vector);

    Vector3 wwv = mve::cross(w, wv);

    wv *= 2.0f * a;

    wwv *= 2.0f;

    Vector3 result = vector;

    result += wv;

    result += wwv;

    return result;
}
inline Vector3 rotate(const Vector3& vector, const Matrix3& matrix)
{
    return matrix * vector;
}
inline Vector3 transform(const Vector3& position, const Matrix4& matrix)
{
    Vector4 result_4d = matrix * Vector4(position.x, position.y, position.z, 1.0f);
    return { result_4d.x, result_4d.y, result_4d.z };
}
inline Vector3::Vector3()
    : x(0.0f)
    , y(0.0f)
    , z(0.0f)
{
}
inline Vector3::Vector3(float val)
    : x(val)
    , y(val)
    , z(val)
{
}
inline Vector3::Vector3(float x, float y, float z)
    : x(x)
    , y(y)
    , z(z)
{
}
inline Vector3 Vector3::zero()
{
    return Vector3(0.0f, 0.0f, 0.0f);
}
inline Vector3 Vector3::one()
{
    return Vector3(1.0f, 1.0f, 1.0f);
}
inline Vector3 Vector3::abs() const
{
    return mve::abs(*this);
}
inline Vector3 Vector3::ceil() const
{
    return mve::ceil(*this);
}
inline Vector3 Vector3::clamp(Vector3 min, Vector3 max) const
{
    return mve::clamp(*this, min, max);
}
inline Vector3 Vector3::direction_to(Vector3 to) const
{
    return mve::direction(*this, to);
}
inline float Vector3::distance_sqrd_to(Vector3 to) const
{
    return mve::distance_sqrd(*this, to);
}
inline float Vector3::distance_to(Vector3 to) const
{
    return mve::distance(*this, to);
}
inline Vector3 Vector3::normalize() const
{
    return mve::normalize(*this);
}
inline Vector3 Vector3::floor() const
{
    return mve::floor(*this);
}
inline float Vector3::length() const
{
    return mve::length(*this);
}
inline float Vector3::length_sqrd() const
{
    return mve::length_sqrd(*this);
}
inline Vector3 Vector3::linear_interpolate(Vector3 to, float weight) const
{
    return mve::linear_interpolate(*this, to, weight);
}
inline Vector3Axis Vector3::max_axis() const
{
    return mve::max_axis(*this);
}
inline Vector3Axis Vector3::min_axis() const
{
    return mve::min_axis(*this);
}
inline bool Vector3::is_equal_approx(Vector3 vector) const
{
    return mve::is_equal_approx(*this, vector);
}
inline bool Vector3::is_zero_approx() const
{
    return mve::is_zero_approx(*this);
}
inline Vector3 Vector3::move_toward(Vector3 to, float amount) const
{
    return mve::move_toward(*this, to, amount);
}
inline float Vector3::dot(Vector3 vector) const
{
    return mve::dot(*this, vector);
}
inline Vector3 Vector3::cross(Vector3 vector) const
{
    return mve::cross(*this, vector);
}
inline Vector3 Vector3::reflect(Vector3 normal) const
{
    return mve::reflect(*this, normal);
}
inline Vector3 Vector3::inverse() const
{
    return mve::inverse(*this);
}
inline Vector3 Vector3::clamp_length(float min, float max) const
{
    return mve::clamp_length(*this, min, max);
}
inline Vector3 Vector3::round() const
{
    return mve::round(*this);
}
inline float Vector3::angle(Vector3 vector) const
{
    return mve::angle(*this, vector);
}
inline Vector3 Vector3::rotate(Vector3 axis, float angle) const
{
    return mve::rotate(*this, axis, angle);
}
inline bool Vector3::operator!=(Vector3 other) const
{
    return x != other.x || y != other.y || z != other.z;
}
inline Vector3 Vector3::operator*(Vector3 other) const
{
    return Vector3(x * other.x, y * other.y, z * other.z);
}
inline Vector3& Vector3::operator*=(Vector3 other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;

    return *this;
}
inline Vector3 Vector3::operator*(float val) const
{
    return Vector3(x * val, y * val, z * val);
}
inline Vector3& Vector3::operator*=(float val)
{
    x *= val;
    y *= val;
    z *= val;

    return *this;
}
inline Vector3 Vector3::operator*(int val) const
{
    return Vector3(x * val, y * val, z * val);
}
inline Vector3& Vector3::operator*=(int val)
{
    x *= val;
    y *= val;
    z *= val;

    return *this;
}
inline Vector3 Vector3::operator+(Vector3 other) const
{
    return Vector3(x + other.x, y + other.y, z + other.z);
}
inline Vector3& Vector3::operator+=(Vector3 other)
{
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}
inline Vector3 Vector3::operator-(const Vector3& other) const
{
    return Vector3(x - other.x, y - other.y, z - other.z);
}
inline Vector3& Vector3::operator-=(const Vector3& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;
}
inline Vector3 Vector3::operator/(Vector3 other) const
{
    return Vector3(x / other.x, y / other.y, z / other.z);
}
inline Vector3& Vector3::operator/=(Vector3 other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;

    return *this;
}
inline Vector3 Vector3::operator/(float val) const
{
    return Vector3(x / val, y / val, z / val);
}
inline Vector3& Vector3::operator/=(float val)
{
    x /= val;
    y /= val;
    z /= val;

    return *this;
}
inline Vector3 Vector3::operator/(int val) const
{
    return Vector3(x / val, y / val, z / val);
}
inline Vector3& Vector3::operator/=(int val)
{
    x /= val;
    y /= val;
    z /= val;

    return *this;
}
inline bool Vector3::operator<(Vector3 other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    if (z != other.z) {
        return z < other.z;
    }
    return false;
}
inline bool Vector3::operator<=(Vector3 other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    if (z != other.z) {
        return z < other.z;
    }
    return true;
}
inline bool Vector3::operator==(Vector3 other) const
{
    return x == other.x && y == other.y && z == other.z;
}
inline bool Vector3::operator>(Vector3 other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    if (z != other.z) {
        return z > other.z;
    }
    return false;
}
inline bool Vector3::operator>=(Vector3 other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    if (z != other.z) {
        return z > other.z;
    }
    return true;
}
inline float& Vector3::operator[](int index)
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        return x;
    }
}
inline float& Vector3::operator[](Vector3Axis axis)
{
    switch (axis) {
    case Vector3Axis::x:
        return x;
    case Vector3Axis::y:
        return y;
    case Vector3Axis::z:
        return z;
    default:
        return x;
    }
}
inline Vector3 Vector3::operator+() const
{
    return Vector3(x, y, z);
}
inline Vector3 Vector3::operator-() const
{
    return Vector3(-x, -y, -z);
}
inline const float& Vector3::operator[](int index) const
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        return x;
    }
}
inline Vector3::Vector3(Vector3i vector)
    : x(static_cast<float>(vector.x))
    , y(static_cast<float>(vector.y))
    , z(static_cast<float>(vector.z))
{
}
inline Vector3 Vector3::rotate(const Matrix3& matrix) const
{
    return mve::rotate(*this, matrix);
}
inline Vector3 Vector3::transform(const Matrix4& matrix) const
{
    return mve::transform(*this, matrix);
}

}