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
    return std::powf(val, 2.0f);
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

}
