#pragma once

#include <functional>

namespace mve {

enum class Vector4iAxis { x, y, z, w };

class Vector4i {
public:
    int x;
    int y;
    int z;
    int w;

    Vector4i();

    //    Vector4i(Vector4 vector);

    Vector4i(int x, int y, int z, int w);

    explicit Vector4i(int val);

    static Vector4i zero();

    static Vector4i one();

    [[nodiscard]] Vector4i abs() const;

    [[nodiscard]] Vector4i clamp(Vector4i min, Vector4i max) const;

    [[nodiscard]] float length() const;

    [[nodiscard]] float length_squared() const;

    [[nodiscard]] Vector4iAxis max_axis() const;

    [[nodiscard]] Vector4iAxis min_axis() const;

    [[nodiscard]] bool operator!=(Vector4i other) const;

    [[nodiscard]] Vector4i operator%(Vector4i other) const;

    Vector4i& operator%=(Vector4i other);

    [[nodiscard]] Vector4i operator%(int val) const;

    Vector4i& operator%=(int val);

    [[nodiscard]] Vector4i operator*(Vector4i other) const;

    Vector4i& operator*=(Vector4i other);

    [[nodiscard]] Vector4i operator*(float val) const;

    Vector4i& operator*=(float val);

    [[nodiscard]] Vector4i operator+(Vector4i other) const;

    Vector4i& operator+=(Vector4i other);

    [[nodiscard]] Vector4i operator-(Vector4i other) const;

    Vector4i& operator-=(Vector4i other);

    [[nodiscard]] Vector4i operator/(Vector4i other) const;

    Vector4i& operator/=(Vector4i other);

    [[nodiscard]] Vector4i operator/(float val) const;

    Vector4i& operator/=(float val);

    [[nodiscard]] Vector4i operator/(int val) const;

    Vector4i& operator/=(int val);

    [[nodiscard]] bool operator<(Vector4i other) const;

    [[nodiscard]] bool operator<=(Vector4i other) const;

    [[nodiscard]] bool operator==(Vector4i other) const;

    [[nodiscard]] bool operator>(Vector4i other) const;

    [[nodiscard]] bool operator>=(Vector4i other) const;

    [[nodiscard]] int& operator[](int index);

    [[nodiscard]] int& operator[](Vector4iAxis axis);

    [[nodiscard]] Vector4i operator+() const;

    [[nodiscard]] Vector4i operator-() const;

    [[nodiscard]] operator bool() const;
};

[[nodiscard]] Vector4i abs(Vector4i vector);

[[nodiscard]] Vector4i clamp(Vector4i vector, Vector4i min, Vector4i max);

[[nodiscard]] float length(Vector4i vector);

[[nodiscard]] float length_squared(Vector4i vector);

[[nodiscard]] Vector4iAxis max_axis(Vector4i vector);

[[nodiscard]] Vector4iAxis min_axis(Vector4i vector);

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