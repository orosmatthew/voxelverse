#pragma once

#include <string>
#include <vector>

#include <GLFW/glfw3.h>

#include <nnm/nnm.hpp>

namespace mve {

class Monitor {
public:
    explicit Monitor(GLFWmonitor* monitor);

    [[nodiscard]] nnm::Vector2i size() const;

    [[nodiscard]] nnm::Vector2i position() const;

    [[nodiscard]] nnm::Vector2i physical_size() const;

    [[nodiscard]] GLFWmonitor* glfw_handle() const;

    [[nodiscard]] int refresh_rate() const;

    [[nodiscard]] std::string name() const;

    [[nodiscard]] static int count();

    [[nodiscard]] static std::vector<Monitor> list();

private:
    GLFWmonitor* m_monitor;
};

}