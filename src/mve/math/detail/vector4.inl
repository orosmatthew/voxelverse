namespace mve {

inline Vector4 abs(Vector4 vector)
{
    return Vector4(abs(vector.x), abs(vector.y), abs(vector.z), abs(vector.w));
}
inline Vector4 ceil(Vector4 vector)
{
    return Vector4(ceil(vector.x), ceil(vector.y), ceil(vector.z), ceil(vector.w));
}
inline Vector4 clamp(Vector4 vector, Vector4 min, Vector4 max)
{
    return Vector4(
        clamp(vector.x, min.x, max.x),
        clamp(vector.y, min.y, max.y),
        clamp(vector.z, min.z, max.z),
        clamp(vector.w, min.w, max.w));
}
inline Vector4 direction(Vector4 from, Vector4 to)
{
    return mve::normalize(to - from);
}
inline float distance_sqrd(Vector4 from, Vector4 to)
{
    float diff_x = to.x - from.x;
    float diff_y = to.y - from.y;
    float diff_z = to.z - from.z;
    float diff_w = to.w - from.w;
    return sqrd(diff_x) + sqrd(diff_y) + sqrd(diff_z) + sqrd(diff_w);
}
inline float distance(Vector4 from, Vector4 to)
{
    float diff_x = to.x - from.x;
    float diff_y = to.y - from.y;
    float diff_z = to.z - from.z;
    float diff_w = to.w - from.w;
    return sqrt(sqrd(diff_x) + sqrd(diff_y) + sqrd(diff_z) + sqrd(diff_w));
}
inline Vector4 normalize(Vector4 vector)
{
    Vector4 result(0.0f);
    float length = mve::length(vector);

    if (length > 0) {
        float inverse_length = 1.0f / length;
        result = vector * inverse_length;
    }

    return result;
}
inline Vector4 floor(Vector4 vector)
{
    return Vector4(floor(vector.x), floor(vector.y), floor(vector.z), floor(vector.w));
}
inline float length(Vector4 vector)
{
    return sqrt(sqrd(vector.x) + sqrd(vector.y) + sqrd(vector.z) + sqrd(vector.w));
}
inline float length_sqrd(Vector4 vector)
{
    return sqrd(vector.x) + sqrd(vector.y) + sqrd(vector.z) + sqrd(vector.w);
}
inline Vector4 linear_interpolate(Vector4 from, Vector4 to, float weight)
{
    return Vector4(
        linear_interpolate(from.x, to.x, weight),
        linear_interpolate(from.y, to.y, weight),
        linear_interpolate(from.z, to.z, weight),
        linear_interpolate(from.w, to.w, weight));
}
inline Vector4Axis min_axis(Vector4 vector)
{
    float min_val = vector.x;
    Vector4Axis min_axis = Vector4Axis::x;

    if (vector.y < min_val) {
        min_val = vector.y;
        min_axis = Vector4Axis::y;
    }

    if (vector.z < min_val) {
        min_val = vector.z;
        min_axis = Vector4Axis::z;
    }

    if (vector.w < min_val) {
        min_axis = Vector4Axis::w;
    }

    return min_axis;
}
inline Vector4Axis max_axis(Vector4 vector)
{
    float max_val = vector.x;
    Vector4Axis max_axis = Vector4Axis::x;

    if (vector.y > max_val) {
        max_val = vector.y;
        max_axis = Vector4Axis::y;
    }

    if (vector.z > max_val) {
        max_val = vector.z;
        max_axis = Vector4Axis::z;
    }

    if (vector.w > max_val) {
        max_axis = Vector4Axis::w;
    }

    return max_axis;
}
inline bool is_equal_approx(Vector4 a, Vector4 b)
{
    return is_equal_approx(a.x, b.x) && is_equal_approx(a.y, b.y) && is_equal_approx(a.z, b.z)
        && is_equal_approx(a.w, b.w);
}
inline bool is_zero_approx(Vector4 vector)
{
    return is_zero_approx(vector.x) && is_zero_approx(vector.y) && is_zero_approx(vector.z) && is_zero_approx(vector.w);
}
inline float dot(Vector4 a, Vector4 b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
}
inline Vector4 inverse(Vector4 vector)
{
    return Vector4(1.0f / vector.x, 1.0f / vector.y, 1.0f / vector.z, 1.0f / vector.w);
}
inline Vector4 clamp_length(Vector4 vector, float min, float max)
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
inline Vector4 round(Vector4 vector)
{
    return Vector4(round(vector.x), round(vector.y), round(vector.z), round(vector.w));
}
inline Vector4::Vector4()
    : x(0.0f)
    , y(0.0f)
    , z(0.0f)
    , w(0.0f)
{
}
inline Vector4::Vector4(float val)
    : x(val)
    , y(val)
    , z(val)
    , w(val)
{
}
inline Vector4::Vector4(float x, float y, float z, float w)
    : x(x)
    , y(y)
    , z(z)
    , w(w)
{
}
inline Vector4 Vector4::zero()
{
    return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}
inline Vector4 Vector4::one()
{
    return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}
inline Vector4 Vector4::abs() const
{
    return mve::abs(*this);
}
inline Vector4 Vector4::ceil() const
{
    return mve::ceil(*this);
}
inline Vector4 Vector4::clamp(Vector4 min, Vector4 max) const
{
    return mve::clamp(*this, min, max);
}
inline Vector4 Vector4::direction_to(Vector4 to) const
{
    return mve::direction(*this, to);
}
inline float Vector4::distance_sqrd_to(Vector4 to) const
{
    return mve::distance_sqrd(*this, to);
}
inline float Vector4::distance_to(Vector4 to) const
{
    return mve::distance(*this, to);
}
inline Vector4 Vector4::normalize() const
{
    return mve::normalize(*this);
}
inline Vector4 Vector4::floor() const
{
    return mve::floor(*this);
}
inline float Vector4::length() const
{
    return mve::length(*this);
}
inline float Vector4::length_sqrd() const
{
    return mve::length_sqrd(*this);
}
inline Vector4 Vector4::linear_interpolate(Vector4 to, float weight) const
{
    return mve::linear_interpolate(*this, to, weight);
}
inline Vector4Axis Vector4::min_axis() const
{
    return mve::min_axis(*this);
}
inline Vector4Axis Vector4::max_axis() const
{
    return mve::max_axis(*this);
}
inline bool Vector4::is_equal_approx(Vector4 vector) const
{
    return mve::is_equal_approx(*this, vector);
}
inline bool Vector4::is_zero_approx() const
{
    return mve::is_zero_approx(*this);
}
inline float Vector4::dot(Vector4 vector) const
{
    return mve::dot(*this, vector);
}
inline Vector4 Vector4::inverse() const
{
    return mve::inverse(*this);
}
inline Vector4 Vector4::clamp_length(float min, float max) const
{
    return mve::clamp_length(*this, min, max);
}
inline Vector4 Vector4::round() const
{
    return mve::round(*this);
}
inline bool Vector4::operator!=(Vector4 other) const
{
    return x != other.x || y != other.y || z != other.z || w != other.w;
}
inline Vector4 Vector4::operator*(Vector4 other) const
{
    return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
}
inline Vector4& Vector4::operator*=(Vector4 other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;

    return *this;
}
inline Vector4 Vector4::operator*(float val) const
{
    return Vector4(x * val, y * val, z * val, w * val);
}
inline Vector4& Vector4::operator*=(float val)
{
    x *= val;
    y *= val;
    z *= val;
    w *= val;

    return *this;
}
inline Vector4 Vector4::operator*(int val) const
{
    return Vector4(x * val, y * val, z * val, w * val);
}
inline Vector4& Vector4::operator*=(int val)
{
    x *= val;
    y *= val;
    z *= val;
    w *= val;

    return *this;
}
inline Vector4 Vector4::operator+(Vector4 other) const
{
    return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}
inline Vector4& Vector4::operator+=(Vector4 other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;

    return *this;
}
inline Vector4 Vector4::operator-(Vector4 other) const
{
    return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}
inline Vector4& Vector4::operator-=(Vector4 other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;

    return *this;
}
inline Vector4 Vector4::operator/(Vector4 other) const
{
    return Vector4(x / other.x, y / other.y, z / other.z, w / other.w);
}
inline Vector4& Vector4::operator/=(Vector4 other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;

    return *this;
}
inline Vector4 Vector4::operator/(float val) const
{
    return Vector4(x / val, y / val, z / val, w / val);
}
inline Vector4& Vector4::operator/=(float val)
{
    x /= val;
    y /= val;
    z /= val;
    w /= val;

    return *this;
}
inline Vector4 Vector4::operator/(int val) const
{
    return Vector4(x / val, y / val, z / val, w / val);
}
inline Vector4& Vector4::operator/=(int val)
{
    x /= val;
    y /= val;
    z /= val;
    w /= val;

    return *this;
}
inline bool Vector4::operator<(Vector4 other) const
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
    if (w != other.w) {
        return w < other.w;
    }
    return false;
}
inline bool Vector4::operator<=(Vector4 other) const
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
    if (w != other.w) {
        return w < other.w;
    }
    return true;
}
inline bool Vector4::operator==(Vector4 other) const
{
    return x == other.x && y == other.y && z == other.z && w == other.w;
}
inline bool Vector4::operator>(Vector4 other) const
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
    if (w != other.w) {
        return w > other.w;
    }
    return false;
}
inline bool Vector4::operator>=(Vector4 other) const
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
    if (w != other.w) {
        return w > other.w;
    }
    return true;
}
inline float& Vector4::operator[](int index)
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    case 3:
        return w;
    default:
        return x;
    }
}
inline float& Vector4::operator[](Vector4Axis axis)
{
    switch (axis) {
    case Vector4Axis::x:
        return x;
    case Vector4Axis::y:
        return y;
    case Vector4Axis::z:
        return z;
    case Vector4Axis::w:
        return w;
    default:
        return x;
    }
}
inline Vector4 Vector4::operator+() const
{
    return Vector4(x, y, z, w);
}
inline Vector4 Vector4::operator-() const
{
    return Vector4(-x, -y, -z, -w);
}
inline const float& Vector4::operator[](int index) const
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    case 3:
        return w;
    default:
        return x;
    }
}

}