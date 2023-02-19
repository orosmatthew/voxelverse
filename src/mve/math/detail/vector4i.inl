namespace mve {

inline Vector4i abs(Vector4i vector)
{
    return Vector4i(abs(vector.x), abs(vector.y), abs(vector.z), abs(vector.w));
}
inline Vector4i clamp(Vector4i vector, Vector4i min, Vector4i max)
{
    return Vector4i(
        clamp(vector.x, min.x, max.x),
        clamp(vector.y, min.y, max.y),
        clamp(vector.z, min.z, max.z),
        clamp(vector.w, min.w, max.w));
}
inline float length(Vector4i vector)
{
    return sqrt(sqrd(vector.x) + sqrd(vector.y) + sqrd(vector.z) + sqrd(vector.w));
}
inline float length_sqrd(Vector4i vector)
{
    return sqrd(vector.x) + sqrd(vector.y) + sqrd(vector.z) + sqrd(vector.w);
}
inline Vector4iAxis max_axis(Vector4i vector)
{
    float max_val = vector.x;
    Vector4iAxis max_axis = Vector4iAxis::x;

    if (vector.y > max_val) {
        max_val = vector.y;
        max_axis = Vector4iAxis::y;
    }

    if (vector.z > max_val) {
        max_val = vector.z;
        max_axis = Vector4iAxis::z;
    }

    if (vector.w > max_val) {
        max_axis = Vector4iAxis::w;
    }

    return max_axis;
}
inline Vector4iAxis min_axis(Vector4i vector)
{
    float min_val = vector.x;
    Vector4iAxis min_axis = Vector4iAxis::x;

    if (vector.y < min_val) {
        min_val = vector.y;
        min_axis = Vector4iAxis::y;
    }

    if (vector.z < min_val) {
        min_val = vector.z;
        min_axis = Vector4iAxis::z;
    }

    if (vector.w < min_val) {
        min_axis = Vector4iAxis::w;
    }

    return min_axis;
}
inline Vector4i::Vector4i()
    : x(0)
    , y(0)
    , z(0)
    , w(0)
{
}
inline Vector4i::Vector4i(int x, int y, int z, int w)
    : x(x)
    , y(y)
    , z(z)
    , w(w)
{
}
inline Vector4i::Vector4i(int val)
    : x(val)
    , y(val)
    , z(val)
    , w(val)
{
}
inline Vector4i Vector4i::zero()
{
    return Vector4i(0, 0, 0, 0);
}
inline Vector4i Vector4i::one()
{
    return Vector4i(1, 1, 1, 1);
}
inline Vector4i Vector4i::abs() const
{
    return mve::abs(*this);
}
inline Vector4i Vector4i::clamp(Vector4i min, Vector4i max) const
{
    return mve::clamp(*this, min, max);
}
inline float Vector4i::length() const
{
    return mve::length(*this);
}
inline float Vector4i::length_sqrd() const
{
    return mve::length_sqrd(*this);
}
inline Vector4iAxis Vector4i::max_axis() const
{
    return mve::max_axis(*this);
}
inline Vector4iAxis Vector4i::min_axis() const
{
    return mve::min_axis(*this);
}
inline bool Vector4i::operator!=(Vector4i other) const
{
    return x != other.x || y != other.y || z != other.z || w != other.w;
}
inline Vector4i Vector4i::operator%(Vector4i other) const
{
    return Vector4i(x % other.x, y & other.y, z % other.z, w % other.w);
}
inline Vector4i& Vector4i::operator%=(Vector4i other)
{
    x %= other.x;
    y %= other.y;
    z %= other.z;
    w %= other.w;

    return *this;
}
inline Vector4i Vector4i::operator%(int val) const
{
    return Vector4i(x % val, y % val, z % val, w % val);
}
inline Vector4i& Vector4i::operator%=(int val)
{
    x %= val;
    y %= val;
    z %= val;
    w %= val;

    return *this;
}
inline Vector4i Vector4i::operator*(Vector4i other) const
{
    return Vector4i(x * other.x, y * other.y, z * other.z, w * other.w);
}
inline Vector4i& Vector4i::operator*=(Vector4i other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;

    return *this;
}
inline Vector4i Vector4i::operator*(float val) const
{
    return Vector4i(x * val, y * val, z * val, w * val);
}
inline Vector4i& Vector4i::operator*=(float val)
{
    x *= val;
    y *= val;
    z *= val;
    w *= val;

    return *this;
}
inline Vector4i Vector4i::operator+(Vector4i other) const
{
    return Vector4i(x + other.x, y + other.y, z + other.z, w + other.w);
}
inline Vector4i& Vector4i::operator+=(Vector4i other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;

    return *this;
}
inline Vector4i Vector4i::operator-(Vector4i other) const
{
    return Vector4i(x - other.x, y - other.y, z - other.z, w - other.w);
}
inline Vector4i& Vector4i::operator-=(Vector4i other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;

    return *this;
}
inline Vector4i Vector4i::operator/(Vector4i other) const
{
    return Vector4i(x / other.x, y / other.y, z / other.z, w / other.w);
}
inline Vector4i& Vector4i::operator/=(Vector4i other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;

    return *this;
}
inline Vector4i Vector4i::operator/(float val) const
{
    return Vector4i(x / val, y / val, z / val, w / val);
}
inline Vector4i& Vector4i::operator/=(float val)
{
    x /= val;
    y /= val;
    z /= val;
    w /= val;

    return *this;
}
inline Vector4i Vector4i::operator/(int val) const
{
    return Vector4i(x / val, y / val, z / val, w / val);
}
inline Vector4i& Vector4i::operator/=(int val)
{
    x /= val;
    y /= val;
    z /= val;
    w /= val;

    return *this;
}
inline bool Vector4i::operator<(Vector4i other) const
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
inline bool Vector4i::operator<=(Vector4i other) const
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
inline bool Vector4i::operator==(Vector4i other) const
{
    return x == other.x && y == other.y && z == other.z && w == other.w;
}
inline bool Vector4i::operator>(Vector4i other) const
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
inline bool Vector4i::operator>=(Vector4i other) const
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
inline int& Vector4i::operator[](int index)
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
inline int& Vector4i::operator[](Vector4iAxis axis)
{
    switch (axis) {
    case Vector4iAxis::x:
        return x;
    case Vector4iAxis::y:
        return y;
    case Vector4iAxis::z:
        return z;
    case Vector4iAxis::w:
        return w;
    default:
        return x;
    }
}
inline Vector4i Vector4i::operator+() const
{
    return Vector4i(x, y, z, w);
}
inline Vector4i Vector4i::operator-() const
{
    return Vector4i(-x, -y, -z, -w);
}
inline Vector4i::operator bool() const
{
    return x != 0 && y != 0 && z != 0 && w != 0;
}
inline Vector4i::Vector4i(const Vector4& vector)
    : x(static_cast<int>(vector.x))
    , y(static_cast<int>(vector.y))
    , z(static_cast<int>(vector.z))
    , w(static_cast<int>(vector.w))
{
}

}

namespace std {
template <>
struct hash<mve::Vector4i> {
    int operator()(const mve::Vector4i& vector) const
    {
        int cantor_z_w = (vector.z + vector.w + 1) * (vector.z + vector.w) / 2 + vector.w;
        int cantor_y_z_w = (vector.y + cantor_z_w + 1) * (vector.y + cantor_z_w) / 2 + cantor_z_w;
        return (vector.x + cantor_y_z_w) * (vector.x + cantor_y_z_w) / 2 + cantor_y_z_w;
    }
};
}
