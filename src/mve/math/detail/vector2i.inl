namespace mve {

inline Vector2i::Vector2i()
    : x(0.0f)
    , y(0.0f)
{
}
inline Vector2i::Vector2i(const Vector2& vector)
    : x(static_cast<int>(vector.x))
    , y(static_cast<int>(vector.y))
{
}
inline Vector2i::Vector2i(int x, int y)
    : x(x)
    , y(y)
{
}
inline Vector2i::Vector2i(int val)
    : x(val)
    , y(val)
{
}
inline Vector2i Vector2i::abs() const
{
    return mve::abs(*this);
}
inline float Vector2i::aspect_ratio() const
{
    return mve::aspect_ratio(*this);
}
inline Vector2i Vector2i::clamp(const Vector2i& min, const Vector2i& max) const
{
    return mve::clamp(*this, min, max);
}
inline float Vector2i::length() const
{
    return mve::length(*this);
}
inline float Vector2i::length_sqrd() const
{
    return mve::length_sqrd(*this);
}
inline Vector2iAxis Vector2i::max_axis() const
{
    return mve::max_axis(*this);
}
inline Vector2iAxis Vector2i::min_axis() const
{
    return mve::min_axis(*this);
}
inline Vector2i Vector2i::zero()
{
    return Vector2i(0.0f, 0.0f);
}
inline Vector2i Vector2i::one()
{
    return Vector2i(1.0f, 1.0f);
}
inline bool Vector2i::operator!=(const Vector2i& other) const
{
    return x != other.x || y != other.y;
}
inline Vector2i Vector2i::operator%(const Vector2i& other) const
{
    return Vector2i(x % other.x, y % other.y);
}
inline Vector2i Vector2i::operator%(int val) const
{
    return Vector2i(x % val, y % val);
}
inline Vector2i Vector2i::operator*(const Vector2i& other) const
{
    return Vector2i(x * other.x, y * other.y);
}
inline Vector2i Vector2i::operator*(float val) const
{
    return Vector2i(x * val, y * val);
}
inline Vector2i Vector2i::operator*(int val) const
{
    return Vector2i(x * val, y * val);
}
inline Vector2i Vector2i::operator+(const Vector2i& other) const
{
    return Vector2i(x + other.x, y + other.y);
}
inline Vector2i Vector2i::operator-(const Vector2i& other) const
{
    return Vector2i(x - other.x, y - other.y);
}
inline Vector2i Vector2i::operator/(const Vector2i& other) const
{
    return Vector2i(x / other.x, y / other.y);
}
inline Vector2i Vector2i::operator/(float val) const
{
    return Vector2i(static_cast<float>(x) / val, static_cast<float>(y) / val);
}
inline Vector2i Vector2i::operator/(int val) const
{
    return Vector2i(x / val, y / val);
}
inline bool Vector2i::operator<(const Vector2i& other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return false;
}
inline bool Vector2i::operator<=(const Vector2i& other) const
{
    if (x != other.x) {
        return x < other.x;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return true;
}
inline bool Vector2i::operator==(const Vector2i& other) const
{
    return x == other.x && y == other.y;
}
inline bool Vector2i::operator>(const Vector2i& other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    return false;
}
inline bool Vector2i::operator>=(const Vector2i& other) const
{
    if (x != other.x) {
        return x > other.x;
    }
    if (y != other.y) {
        return y > other.y;
    }
    return true;
}
inline int& Vector2i::operator[](int index)
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    default:
        return x;
    }
}
inline int& Vector2i::operator[](Vector2iAxis axis)
{
    switch (axis) {
    case Vector2iAxis::x:
        return x;
    case Vector2iAxis::y:
        return y;
    default:
        return x;
    }
}
inline Vector2i Vector2i::operator+() const
{
    return Vector2i(x, y);
}
inline Vector2i Vector2i::operator-() const
{
    return Vector2i(-x, -y);
}
inline Vector2i& Vector2i::operator%=(const Vector2i& other)
{
    x %= other.x;
    y %= other.y;

    return *this;
}
inline Vector2i& Vector2i::operator%=(int val)
{
    x %= val;
    y %= val;

    return *this;
}
inline Vector2i& Vector2i::operator*=(const Vector2i& other)
{
    x *= other.x;
    y *= other.y;

    return *this;
}
inline Vector2i& Vector2i::operator*=(float val)
{
    x *= val;
    y *= val;

    return *this;
}
inline Vector2i& Vector2i::operator*=(int val)
{
    x *= val;
    y *= val;

    return *this;
}
inline Vector2i& Vector2i::operator+=(const Vector2i& other)
{
    x += other.x;
    y += other.y;

    return *this;
}
inline Vector2i& Vector2i::operator-=(const Vector2i& other)
{
    x -= other.x;
    y -= other.y;

    return *this;
}
inline Vector2i& Vector2i::operator/=(const Vector2i& other)
{
    x /= other.x;
    y /= other.y;

    return *this;
}
inline Vector2i& Vector2i::operator/=(float val)
{
    x /= val;
    y /= val;

    return *this;
}
inline Vector2i& Vector2i::operator/=(int val)
{
    x /= val;
    y /= val;

    return *this;
}
inline Vector2i::operator bool() const
{
    return x != 0 || y != 0;
}
inline Vector2i abs(const Vector2i& vector)
{
    return Vector2i(abs(vector.x), abs(vector.y));
}
inline float aspect_ratio(const Vector2i& vector)
{
    return static_cast<float>(vector.x) / static_cast<float>(vector.y);
}
inline Vector2i clamp(const Vector2i& vector, const Vector2i& min, const Vector2i& max)
{
    return Vector2i(clamp(vector.x, min.x, max.x), clamp(vector.y, min.y, max.y));
}
inline float length(const Vector2i& vector)
{
    return sqrt(sqrd(vector.x) + sqrd(vector.y));
}
inline float length_sqrd(const Vector2i& vector)
{
    return sqrd(vector.x) + sqrd(vector.y);
}
inline Vector2iAxis max_axis(const Vector2i& vector)
{
    if (vector.x > vector.y) {
        return Vector2iAxis::x;
    }
    if (vector.y > vector.x) {
        return Vector2iAxis::y;
    }
    return Vector2iAxis::x;
}
inline Vector2iAxis min_axis(const Vector2i& vector)
{
    if (vector.x < vector.y) {
        return Vector2iAxis::x;
    }
    if (vector.y < vector.x) {
        return Vector2iAxis::y;
    }
    return Vector2iAxis::x;
}

}

namespace std {
template <>
struct hash<mve::Vector2i> {
    int operator()(const mve::Vector2i& vector) const
    {
        return (vector.x + vector.y + 1) * (vector.x + vector.y) / 2 + vector.y;
    }
};
}