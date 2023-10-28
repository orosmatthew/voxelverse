#include <cmath>

namespace mve {

inline bool is_zero_approx(float val)
{
    return mve::abs(val) < mve::epsilon;
}

inline bool is_equal_approx(float a, float b)
{
    if (a == b) {
        return true;
    }
    float tolerance = mve::epsilon * mve::abs(a);
    if (tolerance < mve::epsilon) {
        tolerance = mve::epsilon;
    }
    return mve::abs(a - b) < tolerance;
}
inline float abs(float val)
{
    return std::abs(val);
}
inline float ceil(float val)
{
    return std::ceil(val);
}
inline float clamp(float val, float min, float max)
{
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}
inline float sqrt(float val)
{
    return std::sqrt(val);
}
inline float pow(float val, float power)
{
    return std::pow(val, power);
}
inline float sqrd(float val)
{
    return val * val;
}

inline float floor(float val)
{
    return std::floor(val);
}
inline float linear_interpolate(float from, float to, float weight)
{
    return (from * (1.0f - weight)) + (to * weight);
}
inline float sin(float val)
{
    return std::sin(val);
}
inline float cos(float val)
{
    return std::cos(val);
}
inline float tan(float val)
{
    return std::tan(val);
}
inline int abs(int val)
{
    return std::abs(val);
}
inline int clamp(int val, int min, int max)
{
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}
inline int sqrd(int val)
{
    return val * val;
}
inline float round(float val)
{
    return std::round(val);
}
inline float atan(float val)
{
    return std::atan(val);
}
inline float atan2(float a, float b)
{
    return std::atan2(a, b);
}
inline float radians(float degrees)
{
    return degrees * (pi / 180.0f);
}
inline float degrees(float radians)
{
    return radians * (180.0f / pi);
}
inline float asin(float val)
{
    return std::asin(val);
}
inline float acos(float val)
{
    return std::acos(val);
}
inline float min(float a, float b)
{
    if (a <= b) {
        return a;
    }
    else {
        return b;
    }
}
inline float max(float a, float b)
{
    if (a >= b) {
        return a;
    }
    else {
        return b;
    }
}
inline float log2(float val)
{
    return std::log2f(val);
}

}