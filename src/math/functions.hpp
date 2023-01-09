#pragma once

namespace mve {

const float epsilon = 0.000001f;

[[nodiscard]] bool is_equal_approx(float a, float b);

[[nodiscard]] bool is_zero_approx(float val);

[[nodiscard]] float abs(float val);

[[nodiscard]] float ceil(float val);

[[nodiscard]] float clamp(float val, float min, float max);

[[nodiscard]] float sqrt(float val);

[[nodiscard]] float pow(float val, float power);

[[nodiscard]] float squared(float val);

[[nodiscard]] float floor(float val);

}