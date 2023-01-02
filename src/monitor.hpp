#pragma once

#include <vector>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

namespace mve {

class Monitor {
public:
    Monitor(GLFWmonitor* monitor);

    [[nodiscard]] glm::ivec2 size() const;

    [[nodiscard]] glm::ivec2 position() const;

    [[nodiscard]] glm::ivec2 physical_size() const;

    [[nodiscard]] GLFWmonitor* glfw_handle();

    [[nodiscard]] int refresh_rate() const;

    [[nodiscard]] std::string name() const;

    [[nodiscard]] static int count();

    [[nodiscard]] static std::vector<Monitor> list();

private:
    GLFWmonitor* m_monitor;
};

}