#pragma once

#include <cmath>

namespace mve {

inline bool is_zero_approx(const float val)
{
    return abs(val) < epsilon;
}

inline bool is_equal_approx(const float a, const float b)
{
    if (a == b) {
        return true;
    }
    float tolerance = epsilon * abs(a);
    if (tolerance < epsilon) {
        tolerance = epsilon;
    }
    return abs(a - b) < tolerance;
}

inline bool approx_gte(const float a, const float b)
{
    if (a > b) {
        return true;
    }
    float tolerance = epsilon * abs(a);
    if (tolerance < epsilon) {
        tolerance = epsilon;
    }
    return abs(a - b) < tolerance;
}

inline bool approx_lte(const float a, const float b)
{
    if (a < b) {
        return true;
    }
    float tolerance = epsilon * abs(a);
    if (tolerance < epsilon) {
        tolerance = epsilon;
    }
    return abs(a - b) < tolerance;
}

inline float abs(const float val)
{
    return std::abs(val);
}

inline float ceil(const float val)
{
    return std::ceil(val);
}

inline float clamp(const float val, const float min, const float max)
{
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}

inline float sqrt(const float val)
{
    return std::sqrt(val);
}

inline float pow(const float val, const float power)
{
    return std::pow(val, power);
}

inline float sqrd(const float val)
{
    return val * val;
}

inline float floor(const float val)
{
    return std::floor(val);
}

inline float linear_interpolate(const float from, const float to, const float weight)
{
    return from * (1.0f - weight) + to * weight;
}

inline float sin(const float val)
{
    return std::sin(val);
}

inline float cos(const float val)
{
    return std::cos(val);
}

inline float tan(const float val)
{
    return std::tan(val);
}

inline int abs(const int val)
{
    return std::abs(val);
}

inline int clamp(const int val, const int min, const int max)
{
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}

inline int sqrd(const int val)
{
    return val * val;
}

inline float round(const float val)
{
    return std::round(val);
}

inline float atan(const float val)
{
    return std::atan(val);
}

inline float atan2(const float a, const float b)
{
    return std::atan2(a, b);
}

inline float radians(const float degrees)
{
    return degrees * (pi / 180.0f);
}

inline float degrees(const float radians)
{
    return radians * (180.0f / pi);
}

inline float asin(const float val)
{
    return std::asin(val);
}

inline float acos(const float val)
{
    return std::acos(val);
}

inline float min(const float a, const float b)
{
    if (a <= b) {
        return a;
    }
    return b;
}

inline float max(const float a, const float b)
{
    if (a >= b) {
        return a;
    }
    return b;
}

inline float log2(const float val)
{
    return std::log2f(val);
}

}