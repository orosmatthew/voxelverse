#pragma once

#include <string>
#include <vector>

#include "math/math.hpp"
#include "GLFW/glfw3.h"

namespace mve {

class Monitor {
public:
    explicit Monitor(GLFWmonitor* monitor);

    [[nodiscard]] mve::Vector2i size() const;

    [[nodiscard]] mve::Vector2i position() const;

    [[nodiscard]] mve::Vector2i physical_size() const;

    [[nodiscard]] GLFWmonitor* glfw_handle();

    [[nodiscard]] int refresh_rate() const;

    [[nodiscard]] std::string name() const;

    [[nodiscard]] static int count();

    [[nodiscard]] static std::vector<Monitor> list();

private:
    GLFWmonitor* m_monitor;
};

}