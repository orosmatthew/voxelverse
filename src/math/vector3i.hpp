#pragma once

#include <functional>

namespace mve {

enum class Vector3iAxis { x, y, z };

class Vector3;

class Vector3i {
public:
    int x;
    int y;
    int z;

    Vector3i();

    Vector3i(Vector3 vector);

    Vector3i(int x, int y, int z);

    explicit Vector3i(int val);

    static Vector3i zero();

    static Vector3i one();

    [[nodiscard]] Vector3i abs() const;

    [[nodiscard]] Vector3i clamp(Vector3i min, Vector3i max) const;

    [[nodiscard]] float length() const;

    [[nodiscard]] float length_squared() const;

    [[nodiscard]] Vector3iAxis max_axis() const;

    [[nodiscard]] Vector3iAxis min_axis() const;

    [[nodiscard]] bool operator!=(Vector3i other) const;

    [[nodiscard]] Vector3i operator%(Vector3i other) const;

    Vector3i& operator%=(Vector3i other);

    [[nodiscard]] Vector3i operator%(int val) const;

    Vector3i& operator%=(int val);

    [[nodiscard]] Vector3i operator*(Vector3i other) const;

    Vector3i& operator*=(Vector3i other);

    [[nodiscard]] Vector3i operator*(float val) const;

    Vector3i& operator*=(float val);

    [[nodiscard]] Vector3i operator+(Vector3i other) const;

    Vector3i& operator+=(Vector3i other);

    [[nodiscard]] Vector3i operator-(Vector3i other) const;

    Vector3i& operator-=(Vector3i other);

    [[nodiscard]] Vector3i operator/(Vector3i other) const;

    Vector3i& operator/=(Vector3i other);

    [[nodiscard]] Vector3i operator/(float val) const;

    Vector3i& operator/=(float val);

    [[nodiscard]] Vector3i operator/(int val) const;

    Vector3i& operator/=(int val);

    [[nodiscard]] bool operator<(Vector3i other) const;

    [[nodiscard]] bool operator<=(Vector3i other) const;

    [[nodiscard]] bool operator==(Vector3i other) const;

    [[nodiscard]] bool operator>(Vector3i other) const;

    [[nodiscard]] bool operator>=(Vector3i other) const;

    [[nodiscard]] int& operator[](int index);

    [[nodiscard]] int& operator[](Vector3iAxis axis);

    [[nodiscard]] Vector3i operator+() const;

    [[nodiscard]] Vector3i operator-() const;
};

[[nodiscard]] Vector3i abs(Vector3i vector);

[[nodiscard]] Vector3i clamp(Vector3i vector, Vector3i min, Vector3i max);

[[nodiscard]] float length(Vector3i vector);

[[nodiscard]] float length_squared(Vector3i vector);

[[nodiscard]] Vector3iAxis max_axis(Vector3i vector);

[[nodiscard]] Vector3iAxis min_axis(Vector3i vector);

}

namespace std {
template <>
struct hash<mve::Vector3i> {
    int operator()(const mve::Vector3i& vector) const
    {
        int cantor_y_z = (vector.y + vector.z + 1) * (vector.y + vector.z) / 2 + vector.z;
        return (vector.x + cantor_y_z) * (vector.x + cantor_y_z) / 2 + cantor_y_z;
    }
};
}