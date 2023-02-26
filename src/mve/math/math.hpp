#pragma once

#include <functional>

namespace mve {

const float epsilon = 0.00001f;

const float pi = 3.14159265358979323846264338327950288f;

[[nodiscard]] inline bool is_equal_approx(float a, float b);

[[nodiscard]] inline bool is_zero_approx(float val);

[[nodiscard]] inline float abs(float val);

[[nodiscard]] inline int abs(int val);

[[nodiscard]] inline float ceil(float val);

[[nodiscard]] inline float clamp(float val, float min, float max);

[[nodiscard]] inline int clamp(int val, int min, int max);

[[nodiscard]] inline float sqrt(float val);

[[nodiscard]] inline float pow(float val, float power);

[[nodiscard]] inline float sqrd(float val);

[[nodiscard]] inline int squared(int val);

[[nodiscard]] inline float floor(float val);

[[nodiscard]] inline float sin(float val);

[[nodiscard]] inline float asin(float val);

[[nodiscard]] inline float cos(float val);

[[nodiscard]] inline float acos(float val);

[[nodiscard]] inline float tan(float val);

[[nodiscard]] inline float atan(float val);

[[nodiscard]] inline float atan2(float a, float b);

[[nodiscard]] inline float round(float val);

[[nodiscard]] inline float radians(float degrees);

[[nodiscard]] inline float degrees(float radians);

[[nodiscard]] inline float linear_interpolate(float from, float to, float weight);

[[nodiscard]] inline float min(float a, float b);

[[nodiscard]] inline float max(float a, float b);

[[nodiscard]] inline float log2(float val);

class Matrix3;
class Matrix4;
class Quaternion;
class Vector2;
class Vector2i;
class Vector3;
class Vector3i;
class Vector4;
class Vector4i;

enum class Vector4iAxis { x, y, z, w };

class Vector4i {
public:
    int x;
    int y;
    int z;
    int w;

    inline Vector4i();

    inline Vector4i(const Vector4& vector);

    inline Vector4i(int x, int y, int z, int w);

    inline explicit Vector4i(int val);

    static inline Vector4i zero();

    static inline Vector4i one();

    [[nodiscard]] inline Vector4i abs() const;

    [[nodiscard]] inline Vector4i clamp(Vector4i min, Vector4i max) const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector4iAxis max_axis() const;

    [[nodiscard]] inline Vector4iAxis min_axis() const;

    [[nodiscard]] inline bool operator!=(Vector4i other) const;

    [[nodiscard]] inline Vector4i operator%(Vector4i other) const;

    inline Vector4i& operator%=(Vector4i other);

    [[nodiscard]] inline Vector4i operator%(int val) const;

    inline Vector4i& operator%=(int val);

    [[nodiscard]] inline Vector4i operator*(Vector4i other) const;

    inline Vector4i& operator*=(Vector4i other);

    [[nodiscard]] inline Vector4i operator*(float val) const;

    inline Vector4i& operator*=(float val);

    [[nodiscard]] inline Vector4i operator+(Vector4i other) const;

    inline Vector4i& operator+=(Vector4i other);

    [[nodiscard]] inline Vector4i operator-(Vector4i other) const;

    inline Vector4i& operator-=(Vector4i other);

    [[nodiscard]] inline Vector4i operator/(Vector4i other) const;

    inline Vector4i& operator/=(Vector4i other);

    [[nodiscard]] inline Vector4i operator/(float val) const;

    inline Vector4i& operator/=(float val);

    [[nodiscard]] inline Vector4i operator/(int val) const;

    inline Vector4i& operator/=(int val);

    [[nodiscard]] inline bool operator<(Vector4i other) const;

    [[nodiscard]] inline bool operator<=(Vector4i other) const;

    [[nodiscard]] inline bool operator==(Vector4i other) const;

    [[nodiscard]] inline bool operator>(Vector4i other) const;

    [[nodiscard]] inline bool operator>=(Vector4i other) const;

    [[nodiscard]] inline int& operator[](int index);

    [[nodiscard]] inline int& operator[](Vector4iAxis axis);

    [[nodiscard]] inline Vector4i operator+() const;

    [[nodiscard]] inline Vector4i operator-() const;

    [[nodiscard]] inline operator bool() const;
};

[[nodiscard]] inline Vector4i abs(Vector4i vector);

[[nodiscard]] inline Vector4i clamp(Vector4i vector, Vector4i min, Vector4i max);

[[nodiscard]] inline float length(Vector4i vector);

[[nodiscard]] inline float length_sqrd(Vector4i vector);

[[nodiscard]] inline Vector4iAxis max_axis(Vector4i vector);

[[nodiscard]] inline Vector4iAxis min_axis(Vector4i vector);

enum class Vector4Axis { x, y, z, w };

class Vector4 {
public:
    float x;
    float y;
    float z;
    float w;

    inline Vector4();

    inline explicit Vector4(float val);

    inline Vector4(float x, float y, float z, float w);

    static inline Vector4 zero();

    static inline Vector4 one();

    [[nodiscard]] inline Vector4 abs() const;

    [[nodiscard]] inline Vector4 ceil() const;

    [[nodiscard]] inline Vector4 clamp(Vector4 min, Vector4 max) const;

    [[nodiscard]] inline Vector4 direction_to(Vector4 to) const;

    [[nodiscard]] inline float distance_sqrd_to(Vector4 to) const;

    [[nodiscard]] inline float distance_to(Vector4 to) const;

    [[nodiscard]] inline Vector4 normalize() const;

    [[nodiscard]] inline Vector4 floor() const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector4 linear_interpolate(Vector4 to, float weight) const;

    [[nodiscard]] inline Vector4Axis min_axis() const;

    [[nodiscard]] inline Vector4Axis max_axis() const;

    [[nodiscard]] inline bool is_equal_approx(Vector4 vector) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline float dot(Vector4 vector) const;

    [[nodiscard]] inline Vector4 inverse() const;

    [[nodiscard]] inline Vector4 clamp_length(float min, float max) const;

    [[nodiscard]] inline Vector4 round() const;

    [[nodiscard]] inline bool operator!=(Vector4 other) const;

    [[nodiscard]] inline Vector4 operator*(Vector4 other) const;

    inline Vector4& operator*=(Vector4 other);

    [[nodiscard]] inline Vector4 operator*(float val) const;

    inline Vector4& operator*=(float val);

    [[nodiscard]] inline Vector4 operator*(int val) const;

    inline Vector4& operator*=(int val);

    [[nodiscard]] inline Vector4 operator+(Vector4 other) const;

    inline Vector4& operator+=(Vector4 other);

    [[nodiscard]] inline Vector4 operator-(Vector4 other) const;

    inline Vector4& operator-=(Vector4 other);

    [[nodiscard]] inline Vector4 operator/(Vector4 other) const;

    inline Vector4& operator/=(Vector4 other);

    [[nodiscard]] inline Vector4 operator/(float val) const;

    inline Vector4& operator/=(float val);

    [[nodiscard]] inline Vector4 operator/(int val) const;

    inline Vector4& operator/=(int val);

    [[nodiscard]] inline bool operator<(Vector4 other) const;

    [[nodiscard]] inline bool operator<=(Vector4 other) const;

    [[nodiscard]] inline bool operator==(Vector4 other) const;

    [[nodiscard]] inline bool operator>(Vector4 other) const;

    [[nodiscard]] inline bool operator>=(Vector4 other) const;

    [[nodiscard]] inline float& operator[](int index);

    [[nodiscard]] inline const float& operator[](int index) const;

    [[nodiscard]] inline float& operator[](Vector4Axis axis);

    [[nodiscard]] inline Vector4 operator+() const;

    [[nodiscard]] inline Vector4 operator-() const;
};

[[nodiscard]] inline Vector4 abs(Vector4 vector);

[[nodiscard]] inline Vector4 ceil(Vector4 vector);

[[nodiscard]] inline Vector4 clamp(Vector4 vector, Vector4 min, Vector4 max);

[[nodiscard]] inline Vector4 direction(Vector4 from, Vector4 to);

[[nodiscard]] inline float distance_sqrd(Vector4 from, Vector4 to);

[[nodiscard]] inline float distance(Vector4 from, Vector4 to);

[[nodiscard]] inline Vector4 normalize(Vector4 vector);

[[nodiscard]] inline Vector4 floor(Vector4 vector);

[[nodiscard]] inline float length(Vector4 vector);

[[nodiscard]] inline float length_sqrd(Vector4 vector);

[[nodiscard]] inline Vector4 linear_interpolate(Vector4 from, Vector4 to, float weight);

[[nodiscard]] inline Vector4Axis min_axis(Vector4 vector);

[[nodiscard]] inline Vector4Axis max_axis(Vector4 vector);

[[nodiscard]] inline bool is_equal_approx(Vector4 a, Vector4 b);

[[nodiscard]] inline bool is_zero_approx(Vector4 vector);

[[nodiscard]] inline float dot(Vector4 a, Vector4 b);

[[nodiscard]] inline Vector4 inverse(Vector4 vector);

[[nodiscard]] inline Vector4 clamp_length(Vector4 vector, float min, float max);

[[nodiscard]] inline Vector4 round(Vector4 vector);

enum class Vector3iAxis { x, y, z };

class Vector3i {
public:
    int x;
    int y;
    int z;

    inline Vector3i();

    inline Vector3i(Vector3 vector);

    inline Vector3i(int x, int y, int z);

    inline explicit Vector3i(int val);

    static inline Vector3i zero();

    static inline Vector3i one();

    [[nodiscard]] inline Vector3i abs() const;

    [[nodiscard]] inline Vector3i clamp(Vector3i min, Vector3i max) const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector3iAxis max_axis() const;

    [[nodiscard]] inline Vector3iAxis min_axis() const;

    [[nodiscard]] inline bool operator!=(Vector3i other) const;

    [[nodiscard]] inline Vector3i operator%(Vector3i other) const;

    inline Vector3i& operator%=(Vector3i other);

    [[nodiscard]] inline Vector3i operator%(int val) const;

    inline Vector3i& operator%=(int val);

    [[nodiscard]] inline Vector3i operator*(Vector3i other) const;

    inline Vector3i& operator*=(Vector3i other);

    [[nodiscard]] inline Vector3i operator*(float val) const;

    inline Vector3i& operator*=(float val);

    [[nodiscard]] inline Vector3i operator+(const Vector3i& other) const;

    inline Vector3i& operator+=(const Vector3i& other);

    [[nodiscard]] inline Vector3i operator-(Vector3i other) const;

    inline Vector3i& operator-=(Vector3i other);

    [[nodiscard]] inline Vector3i operator/(Vector3i other) const;

    inline Vector3i& operator/=(Vector3i other);

    [[nodiscard]] inline Vector3i operator/(float val) const;

    inline Vector3i& operator/=(float val);

    [[nodiscard]] inline Vector3i operator/(int val) const;

    inline Vector3i& operator/=(int val);

    [[nodiscard]] inline bool operator<(Vector3i other) const;

    [[nodiscard]] inline bool operator<=(Vector3i other) const;

    [[nodiscard]] inline bool operator==(Vector3i other) const;

    [[nodiscard]] inline bool operator>(Vector3i other) const;

    [[nodiscard]] inline bool operator>=(Vector3i other) const;

    [[nodiscard]] inline int& operator[](int index);

    [[nodiscard]] inline int& operator[](Vector3iAxis axis);

    [[nodiscard]] inline Vector3i operator+() const;

    [[nodiscard]] inline Vector3i operator-() const;
};

[[nodiscard]] inline Vector3i abs(Vector3i vector);

[[nodiscard]] inline Vector3i clamp(Vector3i vector, Vector3i min, Vector3i max);

[[nodiscard]] inline float length(Vector3i vector);

[[nodiscard]] inline float length_sqrd(Vector3i vector);

[[nodiscard]] inline Vector3iAxis max_axis(Vector3i vector);

[[nodiscard]] inline Vector3iAxis min_axis(Vector3i vector);

enum class Vector2iAxis { x, y };

class Vector2i {
public:
    int x;
    int y;

    inline Vector2i();

    inline Vector2i(const Vector2& vector);

    inline Vector2i(int x, int y);

    inline explicit Vector2i(int val);

    static inline Vector2i zero();

    static inline Vector2i one();

    [[nodiscard]] inline Vector2i abs() const;

    [[nodiscard]] inline float aspect_ratio() const;

    [[nodiscard]] inline Vector2i clamp(const Vector2i& min, const Vector2i& max) const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector2iAxis max_axis() const;

    [[nodiscard]] inline Vector2iAxis min_axis() const;

    [[nodiscard]] inline bool operator!=(const Vector2i& other) const;

    [[nodiscard]] inline Vector2i operator%(const Vector2i& other) const;

    inline Vector2i& operator%=(const Vector2i& other);

    [[nodiscard]] inline Vector2i operator%(int val) const;

    inline Vector2i& operator%=(int val);

    [[nodiscard]] inline Vector2i operator*(const Vector2i& other) const;

    inline Vector2i& operator*=(const Vector2i& other);

    [[nodiscard]] inline Vector2i operator*(float val) const;

    inline Vector2i& operator*=(float val);

    [[nodiscard]] inline Vector2i operator*(int val) const;

    inline Vector2i& operator*=(int val);

    [[nodiscard]] inline Vector2i operator+(const Vector2i& other) const;

    inline Vector2i& operator+=(const Vector2i& other);

    [[nodiscard]] inline Vector2i operator-(const Vector2i& other) const;

    inline Vector2i& operator-=(const Vector2i& other);

    [[nodiscard]] inline Vector2i operator/(const Vector2i& other) const;

    inline Vector2i& operator/=(const Vector2i& other);

    [[nodiscard]] inline Vector2i operator/(float val) const;

    inline Vector2i& operator/=(float val);

    [[nodiscard]] inline Vector2i operator/(int val) const;

    inline Vector2i& operator/=(int val);

    [[nodiscard]] inline bool operator<(const Vector2i& other) const;

    [[nodiscard]] inline bool operator<=(const Vector2i& other) const;

    [[nodiscard]] inline bool operator==(const Vector2i& other) const;

    [[nodiscard]] inline bool operator>(const Vector2i& other) const;

    [[nodiscard]] inline bool operator>=(const Vector2i& other) const;

    [[nodiscard]] inline int& operator[](int index);

    [[nodiscard]] inline int& operator[](Vector2iAxis axis);

    [[nodiscard]] inline Vector2i operator+() const;

    [[nodiscard]] inline Vector2i operator-() const;

    [[nodiscard]] inline operator bool() const;
};

[[nodiscard]] inline Vector2i abs(const Vector2i& vector);

[[nodiscard]] inline float aspect_ratio(const Vector2i& vector);

[[nodiscard]] inline Vector2i clamp(const Vector2i& vector, const Vector2i& min, const Vector2i& max);

[[nodiscard]] inline float length(const Vector2i& vector);

[[nodiscard]] inline float length_sqrd(const Vector2i& vector);

[[nodiscard]] inline Vector2iAxis max_axis(const Vector2i& vector);

[[nodiscard]] inline Vector2iAxis min_axis(const Vector2i& vector);

enum class Vector2Axis { x, y };

class Vector2 {
public:
    float x;
    float y;

    inline Vector2();

    inline Vector2(const Vector2i& vector);

    inline explicit Vector2(float scalar);

    inline Vector2(float x, float y);

    static inline Vector2 zero();

    static inline Vector2 one();

    [[nodiscard]] inline Vector2 abs() const;

    [[nodiscard]] inline float aspect_ratio() const;

    [[nodiscard]] inline Vector2 ceil() const;

    [[nodiscard]] inline Vector2 clamp(const Vector2& min, const Vector2& max) const;

    [[nodiscard]] inline Vector2 direction_to(const Vector2& to) const;

    [[nodiscard]] inline float direction_sqrd_to(const Vector2& to) const;

    [[nodiscard]] inline float distance_to(const Vector2& to) const;

    [[nodiscard]] inline Vector2 normalize() const;

    [[nodiscard]] inline Vector2 floor() const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector2 linear_interpolate(const Vector2& to, float weight) const;

    [[nodiscard]] inline Vector2Axis max_axis() const;

    [[nodiscard]] inline Vector2Axis min_axis() const;

    [[nodiscard]] inline bool is_equal_approx(const Vector2& vector) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline Vector2 move_toward(const Vector2& to, float amount) const;

    [[nodiscard]] inline float dot(const Vector2& vector) const;

    [[nodiscard]] inline Vector2 reflect(const Vector2& normal) const;

    [[nodiscard]] inline Vector2 rotate(float angle);

    [[nodiscard]] inline Vector2 inverse() const;

    [[nodiscard]] inline Vector2 clamp_length(float min, float max) const;

    [[nodiscard]] inline bool operator!=(const Vector2& other) const;

    [[nodiscard]] inline Vector2 operator*(const Vector2& vector) const;

    inline Vector2& operator*=(const Vector2& vector);

    [[nodiscard]] inline Vector2 operator-(const Vector2& other) const;

    inline Vector2& operator-=(const Vector2& other);

    [[nodiscard]] inline Vector2 operator*(float scalar) const;

    inline Vector2& operator*=(float scalar);

    [[nodiscard]] inline Vector2 operator*(int scalar) const;

    inline Vector2& operator*=(int scalar);

    [[nodiscard]] inline Vector2 operator+(const Vector2& other) const;

    inline Vector2& operator+=(const Vector2& other);

    [[nodiscard]] inline Vector2 operator/(const Vector2& other) const;

    inline Vector2& operator/=(const Vector2& other);

    [[nodiscard]] inline Vector2 operator/(float scalar) const;

    inline Vector2& operator/=(float scalar);

    [[nodiscard]] inline Vector2 operator/(int scalar) const;

    inline Vector2& operator/=(int scalar);

    [[nodiscard]] inline bool operator<(const Vector2& other) const;

    [[nodiscard]] inline bool operator<=(const Vector2& other) const;

    [[nodiscard]] inline bool operator==(const Vector2& other) const;

    [[nodiscard]] inline bool operator>(const Vector2& other) const;

    [[nodiscard]] inline bool operator>=(const Vector2& other) const;

    [[nodiscard]] inline float& operator[](int index);

    [[nodiscard]] inline float& operator[](Vector2Axis axis);

    [[nodiscard]] inline Vector2 operator+() const;

    [[nodiscard]] inline Vector2 operator-() const;

    [[nodiscard]] inline operator bool() const;
};

[[nodiscard]] inline Vector2 abs(const Vector2& vector);

[[nodiscard]] inline float aspect_ratio(const Vector2& vector);

[[nodiscard]] inline Vector2 ceil(const Vector2& vector);

[[nodiscard]] inline Vector2 clamp(const Vector2& vector, const Vector2& min, const Vector2& max);

[[nodiscard]] inline Vector2 direction(const Vector2& from, const Vector2& to);

[[nodiscard]] inline float distance_sqrd(const Vector2& from, const Vector2& to);

[[nodiscard]] inline float distance(const Vector2& from, const Vector2& to);

[[nodiscard]] inline Vector2 normalize(const Vector2& vector);

[[nodiscard]] inline Vector2 floor(const Vector2& vector);

[[nodiscard]] inline float length(const Vector2& vector);

[[nodiscard]] inline float length_sqrd(const Vector2& vector);

[[nodiscard]] inline Vector2 linear_interpolate(const Vector2& from, const Vector2& to, float weight);

[[nodiscard]] inline Vector2Axis max_axis(const Vector2& vector);

[[nodiscard]] inline Vector2Axis min_axis(const Vector2& vector);

[[nodiscard]] inline bool is_equal_approx(const Vector2& a, const Vector2& b);

[[nodiscard]] inline bool is_zero_approx(const Vector2& vector);

[[nodiscard]] inline Vector2 move_toward(const Vector2& from, const Vector2& to, float amount);

[[nodiscard]] inline float dot(const Vector2& a, const Vector2& b);

[[nodiscard]] inline Vector2 reflect(const Vector2& vector, const Vector2& normal);

[[nodiscard]] inline Vector2 rotate(const Vector2& vector, float angle);

[[nodiscard]] inline Vector2 inverse(const Vector2& vector);

[[nodiscard]] inline Vector2 clamp_length(const Vector2& vector, float min, float max);

class Quaternion {
public:
    union {
        Vector4 data;
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };

    inline Quaternion();

    inline Quaternion(const Vector4& vector);

    inline Quaternion(float x, float y, float z, float w);

    [[nodiscard]] inline static Quaternion from_axis_angle(const Vector3& axis, float angle);

    [[nodiscard]] inline static Quaternion from_euler(const Vector3& euler);

    [[nodiscard]] inline static Quaternion from_matrix(const Matrix3& matrix);

    [[nodiscard]] inline static Quaternion from_vector3_to_vector3(const Vector3& from, const Vector3& to);

    [[nodiscard]] inline static Quaternion from_direction(const Vector3& dir, const Vector3& up);

    [[nodiscard]] inline float angle_to(const Quaternion& to) const;

    [[nodiscard]] inline float dot(const Quaternion& quaternion) const;

    [[nodiscard]] inline Vector3 euler() const;

    [[nodiscard]] inline Quaternion inverse() const;

    [[nodiscard]] inline bool is_equal_approx(const Quaternion& quaternion) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Quaternion spherical_linear_interpolate(const Quaternion& to, float weight) const;

    [[nodiscard]] inline Matrix3 matrix() const;

    [[nodiscard]] inline Quaternion normalize() const;

    [[nodiscard]] inline bool operator!=(const Quaternion& other) const;

    [[nodiscard]] inline Quaternion operator*(const Quaternion& other) const;

    inline void operator*=(const Quaternion& other);

    [[nodiscard]] inline Quaternion operator*(float value) const;

    inline void operator*=(float value);

    [[nodiscard]] inline Quaternion operator*(int value) const;

    inline void operator*=(int value);

    [[nodiscard]] inline Quaternion operator+(const Quaternion& other) const;

    inline void operator+=(const Quaternion& other);

    [[nodiscard]] inline Quaternion operator-(const Quaternion& other) const;

    inline void operator-=(const Quaternion& other);

    [[nodiscard]] inline Quaternion operator/(float value) const;

    inline void operator/=(float value);

    [[nodiscard]] inline Quaternion operator/(int value) const;

    inline void operator/=(int value);

    [[nodiscard]] inline bool operator==(const Quaternion& other) const;

    [[nodiscard]] inline float& operator[](int index);

    [[nodiscard]] inline const float& operator[](int index) const;

    [[nodiscard]] inline Quaternion operator+() const;

    [[nodiscard]] inline Quaternion operator-() const;
};

[[nodiscard]] inline float angle(const Quaternion& from, const Quaternion& to);

[[nodiscard]] inline float dot(const Quaternion& a, const Quaternion& b);

[[nodiscard]] inline Vector3 euler(const Quaternion& quaternion);

[[nodiscard]] inline Quaternion inverse(const Quaternion& quaternion);

[[nodiscard]] inline bool is_equal_approx(const Quaternion& a, const Quaternion& b);

[[nodiscard]] inline bool is_zero_approx(const Quaternion& quaternion);

[[nodiscard]] inline float length(const Quaternion& quaternion);

[[nodiscard]] inline float length_sqrd(const Quaternion& quaternion);

[[nodiscard]] inline Quaternion spherical_linear_interpolate(
    const Quaternion& from, const Quaternion& to, float weight);

[[nodiscard]] inline Quaternion normalize(const Quaternion& quaternion);

class Matrix4 {
public:
    Vector4 col0;
    Vector4 col1;
    Vector4 col2;
    Vector4 col3;

    inline Matrix4();

    inline Matrix4(const Vector4& col0, const Vector4& col1, const Vector4& col2, const Vector4& col3);

    [[nodiscard]] inline static Matrix4 zero();

    [[nodiscard]] inline static Matrix4 identity();

    [[nodiscard]] inline static Matrix4 from_rotation_translation(
        const Vector3& rotation_x, const Vector3& rotation_y, const Vector3& rotation_z, const Vector3& translation);

    [[nodiscard]] inline static Matrix4 from_basis_translation(const Matrix3& basis, const Vector3& translation);

    [[nodiscard]] inline float determinant() const;

    [[nodiscard]] inline float trace() const;

    [[nodiscard]] inline Matrix4 transpose() const;

    [[nodiscard]] inline Matrix4 inverse() const;

    [[nodiscard]] inline Matrix4 interpolate(const Matrix4& to, float weight) const;

    [[nodiscard]] inline Matrix4 rotate(const Vector3& axis, float angle) const;

    [[nodiscard]] inline Matrix4 rotate(const Matrix3& basis) const;

    [[nodiscard]] inline Matrix4 rotate_local(const Vector3& axis, float angle) const;

    [[nodiscard]] inline Matrix4 rotate_local(const Matrix3& basis) const;

    [[nodiscard]] inline Matrix4 scale(const Vector3& scale) const;

    [[nodiscard]] inline Matrix4 translate(const Vector3& offset) const;

    [[nodiscard]] inline Matrix4 translate_local(const Vector3& offset) const;

    [[nodiscard]] inline bool is_equal_approx(const Matrix4& matrix) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline Matrix3 basis() const;

    [[nodiscard]] inline Vector3 translation() const;

    [[nodiscard]] inline Quaternion quaternion() const;

    [[nodiscard]] inline Vector3 scale() const;

    [[nodiscard]] inline Vector3 euler() const;

    inline Vector4& operator[](int index);

    inline const Vector4& operator[](int index) const;

    [[nodiscard]] inline Matrix4 operator+(const Matrix4& other) const;

    inline void operator+=(const Matrix4& other);

    [[nodiscard]] inline Matrix4 operator-(const Matrix4& other) const;

    inline void operator-=(const Matrix4& other);

    [[nodiscard]] inline Matrix4 operator*(const Matrix4& other) const;

    inline void operator*=(const Matrix4& other);

    [[nodiscard]] inline Vector4 operator*(const Vector4& vector) const;
};

[[nodiscard]] inline float determinant(Matrix4 matrix);

[[nodiscard]] inline float trace(Matrix4 matrix);

[[nodiscard]] inline Matrix4 transpose(const Matrix4& matrix);

[[nodiscard]] inline Matrix4 inverse(Matrix4 matrix);

[[nodiscard]] inline Matrix4 interpolate(Matrix4 from, Matrix4 to, float weight);

[[nodiscard]] inline Matrix4 rotate(Matrix4 matrix, Vector3 axis, float angle);

[[nodiscard]] inline Matrix4 rotate(const Matrix4& matrix, const Matrix3& basis);

[[nodiscard]] inline Matrix4 rotate_local(const Matrix4& matrix, const Vector3& axis, float angle);

[[nodiscard]] inline Matrix4 rotate_local(const Matrix4& matrix, const Matrix3& basis);

[[nodiscard]] inline Matrix4 scale(Matrix4 matrix, Vector3 scale);

[[nodiscard]] inline Matrix4 translate(Matrix4 matrix, Vector3 offset);

[[nodiscard]] inline Matrix4 translate_local(const Matrix4& matrix, Vector3 offset);

[[nodiscard]] inline bool is_equal_approx(Matrix4 a, Matrix4 b);

[[nodiscard]] inline bool is_zero_approx(Matrix4 matrix);

[[nodiscard]] inline Matrix3 basis(const Matrix4& matrix);

[[nodiscard]] inline Matrix4 frustum(float left, float right, float bottom, float top, float near, float far);

[[nodiscard]] inline Matrix4 perspective(float fov_y, float aspect, float near, float far);

[[nodiscard]] inline Matrix4 ortho(float left, float right, float bottom, float top, float near, float far);

[[nodiscard]] inline Matrix4 look_at(Vector3 eye, Vector3 target, Vector3 up);

[[nodiscard]] inline Vector3 translation(const Matrix4& matrix);

[[nodiscard]] inline Vector3 scale(const Matrix4& matrix);

[[nodiscard]] inline Vector3 euler(const Matrix4& matrix);

enum class Vector3Axis { x, y, z };

class Vector3 {
public:
    float x;
    float y;
    float z;

    inline Vector3();

    inline Vector3(Vector3i vector);

    inline explicit Vector3(float val);

    inline Vector3(float x, float y, float z);

    static inline Vector3 zero();

    static inline Vector3 one();

    [[nodiscard]] inline Vector3 abs() const;

    [[nodiscard]] inline Vector3 ceil() const;

    [[nodiscard]] inline Vector3 clamp(Vector3 min, Vector3 max) const;

    [[nodiscard]] inline Vector3 direction_to(Vector3 to) const;

    [[nodiscard]] inline float distance_sqrd_to(Vector3 to) const;

    [[nodiscard]] inline float distance_to(Vector3 to) const;

    [[nodiscard]] inline Vector3 normalize() const;

    [[nodiscard]] inline Vector3 floor() const;

    [[nodiscard]] inline float length() const;

    [[nodiscard]] inline float length_sqrd() const;

    [[nodiscard]] inline Vector3 linear_interpolate(Vector3 to, float weight) const;

    [[nodiscard]] inline Vector3Axis max_axis() const;

    [[nodiscard]] inline Vector3Axis min_axis() const;

    [[nodiscard]] inline bool is_equal_approx(Vector3 vector) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline Vector3 move_toward(Vector3 to, float amount) const;

    [[nodiscard]] inline float dot(Vector3 vector) const;

    [[nodiscard]] inline Vector3 cross(Vector3 vector) const;

    [[nodiscard]] inline Vector3 reflect(Vector3 normal) const;

    [[nodiscard]] inline Vector3 inverse() const;

    [[nodiscard]] inline Vector3 clamp_length(float min, float max) const;

    [[nodiscard]] inline Vector3 round() const;

    [[nodiscard]] inline float angle(Vector3 vector) const;

    [[nodiscard]] inline Vector3 rotate(Vector3 axis, float angle) const;

    [[nodiscard]] inline Vector3 rotate(const Matrix3& matrix) const;

    [[nodiscard]] inline Vector3 transform(const Matrix4& matrix) const;

    [[nodiscard]] inline bool operator!=(Vector3 other) const;

    [[nodiscard]] inline Vector3 operator*(Vector3 other) const;

    inline Vector3& operator*=(Vector3 other);

    [[nodiscard]] inline Vector3 operator*(float val) const;

    inline Vector3& operator*=(float val);

    [[nodiscard]] inline Vector3 operator*(int val) const;

    inline Vector3& operator*=(int val);

    [[nodiscard]] inline Vector3 operator+(Vector3 other) const;

    inline Vector3& operator+=(Vector3 other);

    [[nodiscard]] inline Vector3 operator-(const Vector3& other) const;

    inline Vector3& operator-=(const Vector3& other);

    [[nodiscard]] inline Vector3 operator/(Vector3 other) const;

    inline Vector3& operator/=(Vector3 other);

    [[nodiscard]] inline Vector3 operator/(float val) const;

    inline Vector3& operator/=(float val);

    [[nodiscard]] inline Vector3 operator/(int val) const;

    inline Vector3& operator/=(int val);

    [[nodiscard]] inline bool operator<(Vector3 other) const;

    [[nodiscard]] inline bool operator<=(Vector3 other) const;

    [[nodiscard]] inline bool operator==(Vector3 other) const;

    [[nodiscard]] inline bool operator>(Vector3 other) const;

    [[nodiscard]] inline bool operator>=(Vector3 other) const;

    [[nodiscard]] inline float& operator[](int index);

    [[nodiscard]] inline const float& operator[](int index) const;

    [[nodiscard]] inline float& operator[](Vector3Axis axis);

    [[nodiscard]] inline Vector3 operator+() const;

    [[nodiscard]] inline Vector3 operator-() const;
};

[[nodiscard]] inline Vector3 abs(Vector3 vector);

[[nodiscard]] inline Vector3 ceil(Vector3 vector);

[[nodiscard]] inline Vector3 clamp(Vector3 vector, Vector3 min, Vector3 max);

[[nodiscard]] inline Vector3 direction(Vector3 from, Vector3 to);

[[nodiscard]] inline float distance_sqrd(Vector3 from, Vector3 to);

[[nodiscard]] inline float distance(Vector3 from, Vector3 to);

[[nodiscard]] inline Vector3 normalize(Vector3 vector);

[[nodiscard]] inline Vector3 floor(Vector3 vector);

[[nodiscard]] inline float length(Vector3 vector);

[[nodiscard]] inline float length_sqrd(Vector3 vector);

[[nodiscard]] inline Vector3 linear_interpolate(Vector3 from, Vector3 to, float weight);

[[nodiscard]] inline Vector3Axis max_axis(Vector3 vector);

[[nodiscard]] inline Vector3Axis min_axis(Vector3 vector);

[[nodiscard]] inline bool is_equal_approx(Vector3 a, Vector3 b);

[[nodiscard]] inline bool is_zero_approx(Vector3 vector);

[[nodiscard]] inline Vector3 move_toward(Vector3 from, Vector3 to, float amount);

[[nodiscard]] inline float dot(Vector3 a, Vector3 b);

[[nodiscard]] inline Vector3 cross(Vector3 a, Vector3 b);

[[nodiscard]] inline Vector3 reflect(Vector3 vector, Vector3 normal);

[[nodiscard]] inline Vector3 inverse(Vector3 vector);

[[nodiscard]] inline Vector3 clamp_length(Vector3 vector, float min, float max);

[[nodiscard]] inline Vector3 round(Vector3 vector);

[[nodiscard]] inline float angle(Vector3 a, Vector3 b);

[[nodiscard]] inline Vector3 rotate(Vector3 vector, Vector3 axis, float angle);

[[nodiscard]] inline Vector3 rotate(const Vector3& vector, const Matrix3& matrix);

[[nodiscard]] inline Vector3 transform(const Vector3& position, const Matrix4& matrix);

class Matrix3 {
public:
    Vector3 col0;
    Vector3 col1;
    Vector3 col2;

    inline Matrix3();

    inline Matrix3(const Vector3& col0, const Vector3& col1, const Vector3& col2);

    inline Matrix3(
        float c0r0, float c0r1, float c0r2, float c1r0, float c1r1, float c1r2, float c2r0, float c2r1, float c2r2);

    [[nodiscard]] inline static Matrix3 from_axis_angle(const Vector3& axis, float angle);

    [[nodiscard]] inline static Matrix3 from_euler(const Vector3& euler);

    [[nodiscard]] inline static Matrix3 zero();

    [[nodiscard]] inline static Matrix3 identity();

    [[nodiscard]] inline static Matrix3 from_quaternion(const Quaternion& quaternion);

    [[nodiscard]] inline static Matrix3 from_quaternion_scale(const Quaternion& quaternion, const Vector3& scale);

    [[nodiscard]] inline static Matrix3 from_scale(const Vector3& scale);

    [[nodiscard]] inline static Matrix3 look_at(const Vector3& target, const Vector3& up);

    [[nodiscard]] inline static Matrix3 from_matrix(const Matrix4& matrix);

    [[nodiscard]] inline static Matrix3 from_direction(const Vector3& dir, const Vector3& up);

    [[nodiscard]] inline Vector3 euler() const;

    [[nodiscard]] inline Vector3 scale() const;

    [[nodiscard]] inline float determinant() const;

    [[nodiscard]] inline float trace() const;

    [[nodiscard]] inline Matrix3 transpose() const;

    [[nodiscard]] inline Matrix3 inverse() const;

    [[nodiscard]] inline Matrix3 spherical_linear_interpolate(const Matrix3& to, float weight) const;

    [[nodiscard]] inline Matrix3 rotate(const Vector3& axis, float angle) const;

    [[nodiscard]] inline Matrix3 rotate(const Quaternion& quaternion) const;

    [[nodiscard]] inline Matrix3 scale(const Vector3& scale) const;

    [[nodiscard]] inline Matrix3 orthonormalize() const;

    [[nodiscard]] inline bool is_equal_approx(const Matrix3& matrix) const;

    [[nodiscard]] inline bool is_zero_approx() const;

    [[nodiscard]] inline Quaternion quaternion() const;

    [[nodiscard]] inline Matrix4 with_translation(const Vector3 translation) const;

    inline Vector3& operator[](int index);

    inline const Vector3& operator[](int index) const;

    [[nodiscard]] inline Matrix3 operator+(const Matrix3& other) const;

    inline void operator+=(const Matrix3& other);

    [[nodiscard]] inline Matrix3 operator-(const Matrix3& other) const;

    inline void operator-=(const Matrix3& other);

    [[nodiscard]] inline Matrix3 operator*(const Matrix3& other) const;

    inline void operator*=(const Matrix3& other);

    [[nodiscard]] inline Matrix3 operator*(float val) const;

    inline void operator*=(float val);

    [[nodiscard]] inline Matrix3 operator*(int val) const;

    inline void operator*=(int val);

    [[nodiscard]] inline Vector3 operator*(Vector3 vector) const;

    [[nodiscard]] inline bool operator!=(const Matrix3& other) const;

    [[nodiscard]] inline bool operator==(const Matrix3& other) const;

    [[nodiscard]] inline bool operator<(const Matrix3& other) const;

    [[nodiscard]] inline bool operator<=(const Matrix3& other) const;

    [[nodiscard]] inline bool operator>(const Matrix3& other) const;

    [[nodiscard]] inline bool operator>=(const Matrix3& other) const;
};

[[nodiscard]] inline Vector3 euler(const Matrix3& matrix);

[[nodiscard]] inline Vector3 scale(const Matrix3& matrix);

[[nodiscard]] inline float determinant(const Matrix3& matrix);

[[nodiscard]] inline float trace(const Matrix3& matrix);

[[nodiscard]] inline Matrix3 transpose(const Matrix3& matrix);

[[nodiscard]] inline Matrix3 inverse(const Matrix3& matrix);

[[nodiscard]] inline Matrix3 spherical_linear_interpolate(const Matrix3& from, const Matrix3& to, float weight);

[[nodiscard]] inline Matrix3 rotate(const Matrix3& matrix, const Vector3& axis, float angle);

[[nodiscard]] inline Matrix3 rotate(const Matrix3& matrix, const Quaternion& quaternion);

[[nodiscard]] inline Matrix3 scale(const Matrix3& matrix, const Vector3& scale);

[[nodiscard]] inline Matrix3 orthonormalize(const Matrix3& matrix);

[[nodiscard]] inline bool is_equal_approx(const Matrix3& a, const Matrix3& b);

[[nodiscard]] inline bool is_zero_approx(const Matrix3& matrix);

[[nodiscard]] inline Matrix4 with_translation(const Matrix3& matrix, const Vector3 translation);

[[nodiscard]] inline Matrix3 look_at(const Vector3& target, const Vector3& up);

}

#include "detail/functions.inl"
#include "detail/matrix3.inl"
#include "detail/matrix4.inl"
#include "detail/quaternion.inl"
#include "detail/vector2.inl"
#include "detail/vector2i.inl"
#include "detail/vector3.inl"
#include "detail/vector3i.inl"
#include "detail/vector4.inl"
#include "detail/vector4i.inl"