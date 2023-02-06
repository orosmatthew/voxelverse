#include <cmath>

namespace mve {

inline bool is_zero_approx(float val)
{
    return std::fabsf(val) < epsilon;
}

inline bool is_equal_approx(float a, float b)
{
    if (a == b) {
        return true;
    }
    return is_zero_approx(a - b);
}
inline float abs(float val)
{
    return std::abs(val);
}
inline float ceil(float val)
{
    return std::ceilf(val);
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
    return std::sqrtf(val);
}
inline float pow(float val, float power)
{
    return std::powf(val, power);
}
inline float squared(float val)
{
    return val * val;
}

inline float floor(float val)
{
    return std::floorf(val);
}
inline float linear_interpolate(float from, float to, float weight)
{
    return (from * (1.0f - weight)) + (to * weight);
}
inline float sin(float val)
{
    return std::sinf(val);
}
inline float cos(float val)
{
    return std::cosf(val);
}
inline float tan(float val)
{
    return std::tanf(val);
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
inline int squared(int val)
{
    return val * val;
}
inline float round(float val)
{
    return std::round(val);
}
inline float atan(float val)
{
    return std::atanf(val);
}
inline float atan2(float a, float b)
{
    return std::atan2f(a, b);
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
    return std::asinf(val);
}
inline float acos(float val)
{
    return std::acosf(val);
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