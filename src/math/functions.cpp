#include "functions.hpp"

#include <cmath>

namespace mve {

bool is_zero_approx(float val)
{
    return std::fabsf(val) < epsilon;
}

bool is_equal_approx(float a, float b)
{
    if (a == b) {
        return true;
    }
    return is_zero_approx(a - b);
}
float abs(float val)
{
    return std::abs(val);
}
float ceil(float val)
{
    return std::ceilf(val);
}
float clamp(float val, float min, float max)
{
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}
float sqrt(float val)
{
    return std::sqrtf(val);
}
float pow(float val, float power)
{
    return std::powf(val, power);
}
float squared(float val)
{
    return val * val;
}

float floor(float val)
{
    return std::floorf(val);
}
float linear_interpolate(float from, float to, float weight)
{
    return (from * (1.0f - weight)) + (to * weight);
}
float sin(float val)
{
    return std::sinf(val);
}
float cos(float val)
{
    return std::cosf(val);
}
float tan(float val)
{
    return std::tanf(val);
}
int abs(int val)
{
    return std::abs(val);
}
int clamp(int val, int min, int max)
{
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}
int squared(int val)
{
    return val * val;
}
float round(float val)
{
    return std::round(val);
}
float atan(float val)
{
    return std::atanf(val);
}
float atan2(float a, float b)
{
    return std::atan2f(a, b);
}
float radians(float degrees)
{
    return degrees * (pi / 180.0f);
}
float degrees(float radians)
{
    return radians * (180.0f / pi);
}
float asin(float val)
{
    return std::asinf(val);
}
float acos(float val)
{
    return std::acosf(val);
}
float min(float a, float b)
{
    if (a <= b) {
        return a;
    }
    else {
        return b;
    }
}
float max(float a, float b)
{
    if (a >= b) {
        return a;
    }
    else {
        return b;
    }
}
float log2(float val)
{
    return std::log2f(val);
}

}