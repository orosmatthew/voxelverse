#pragma once

#include <functional>

namespace mve {

enum class Vector2iAxis { x, y };

class Vector2;

class Vector2i {
public:
    int x;
    int y;

    Vector2i();

    Vector2i(const Vector2& vector);

    Vector2i(int x, int y);

    explicit Vector2i(int val);

    static Vector2i zero();

    static Vector2i one();

    [[nodiscard]] Vector2i abs() const;

    [[nodiscard]] float aspect_ratio() const;

    [[nodiscard]] Vector2i clamp(const Vector2i& min, const Vector2i& max) const;

    [[nodiscard]] float length() const;

    [[nodiscard]] float length_squared() const;

    [[nodiscard]] Vector2iAxis max_axis() const;

    [[nodiscard]] Vector2iAxis min_axis() const;

    [[nodiscard]] bool operator!=(const Vector2i& other) const;

    [[nodiscard]] Vector2i operator%(const Vector2i& other) const;

    Vector2i& operator%=(const Vector2i& other);

    [[nodiscard]] Vector2i operator%(int val) const;

    Vector2i& operator%=(int val);

    [[nodiscard]] Vector2i operator*(const Vector2i& other) const;

    Vector2i& operator*=(const Vector2i& other);

    [[nodiscard]] Vector2i operator*(float val) const;

    Vector2i& operator*=(float val);

    [[nodiscard]] Vector2i operator*(int val) const;

    Vector2i& operator*=(int val);

    [[nodiscard]] Vector2i operator+(const Vector2i& other) const;

    Vector2i& operator+=(const Vector2i& other);

    [[nodiscard]] Vector2i operator-(const Vector2i& other) const;

    Vector2i& operator-=(const Vector2i& other);

    [[nodiscard]] Vector2i operator/(const Vector2i& other) const;

    Vector2i& operator/=(const Vector2i& other);

    [[nodiscard]] Vector2i operator/(float val) const;

    Vector2i& operator/=(float val);

    [[nodiscard]] Vector2i operator/(int val) const;

    Vector2i& operator/=(int val);

    [[nodiscard]] bool operator<(const Vector2i& other) const;

    [[nodiscard]] bool operator<=(const Vector2i& other) const;

    [[nodiscard]] bool operator==(const Vector2i& other) const;

    [[nodiscard]] bool operator>(const Vector2i& other) const;

    [[nodiscard]] bool operator>=(const Vector2i& other) const;

    [[nodiscard]] int& operator[](int index);

    [[nodiscard]] int& operator[](Vector2iAxis axis);

    [[nodiscard]] Vector2i operator+() const;

    [[nodiscard]] Vector2i operator-() const;

    [[nodiscard]] operator bool() const;
};

[[nodiscard]] Vector2i abs(const Vector2i& vector);

[[nodiscard]] float aspect_ratio(const Vector2i& vector);

[[nodiscard]] Vector2i clamp(const Vector2i& vector, const Vector2i& min, const Vector2i& max);

[[nodiscard]] float length(const Vector2i& vector);

[[nodiscard]] float length_squared(const Vector2i& vector);

[[nodiscard]] Vector2iAxis max_axis(const Vector2i& vector);

[[nodiscard]] Vector2iAxis min_axis(const Vector2i& vector);

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