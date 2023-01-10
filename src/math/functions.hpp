#pragma once

namespace mve {

const float epsilon = 0.000001f;

const float pi = 3.14159265358979323846264338327950288f;

[[nodiscard]] bool is_equal_approx(float a, float b);

[[nodiscard]] bool is_zero_approx(float val);

[[nodiscard]] float abs(float val);

[[nodiscard]] int abs(int val);

[[nodiscard]] float ceil(float val);

[[nodiscard]] float clamp(float val, float min, float max);

[[nodiscard]] int clamp(int val, int min, int max);

[[nodiscard]] float sqrt(float val);

[[nodiscard]] float pow(float val, float power);

[[nodiscard]] float squared(float val);

[[nodiscard]] int squared(int val);

[[nodiscard]] float floor(float val);

[[nodiscard]] float sin(float val);

[[nodiscard]] float asin(float val);

[[nodiscard]] float cos(float val);

[[nodiscard]] float acos(float val);

[[nodiscard]] float tan(float val);

[[nodiscard]] float atan(float val);

[[nodiscard]] float atan2(float a, float b);

[[nodiscard]] float round(float val);

[[nodiscard]] float radians(float degrees);

[[nodiscard]] float degrees(float radians);

[[nodiscard]] float linear_interpolate(float from, float to, float weight);

}