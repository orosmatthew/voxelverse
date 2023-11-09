#include <mve/monitor.hpp>

mve::Monitor::Monitor(GLFWmonitor* monitor)
    : m_monitor(monitor)
{
}
int mve::Monitor::count()
{
    int monitor_count;
    glfwGetMonitors(&monitor_count);
    return monitor_count;
}

mve::Vector2i mve::Monitor::size() const
{
    const GLFWvidmode* mode = glfwGetVideoMode(m_monitor);
    return { mode->width, mode->height };
}

mve::Vector2i mve::Monitor::position() const
{
    Vector2i pos;
    glfwGetMonitorPos(m_monitor, &pos.x, &pos.y);
    return pos;
}
mve::Vector2i mve::Monitor::physical_size() const
{
    Vector2i physical_size;
    glfwGetMonitorPhysicalSize(m_monitor, &physical_size.x, &physical_size.y);
    return physical_size;
}

GLFWmonitor* mve::Monitor::glfw_handle() const
{
    return m_monitor;
}

std::vector<mve::Monitor> mve::Monitor::list()
{
    int monitor_count;
    GLFWmonitor** glfw_monitors = glfwGetMonitors(&monitor_count);
    std::vector<Monitor> monitors;
    monitors.reserve(monitor_count);

    for (int i = 0; i < monitor_count; i++) {
        monitors.emplace_back(glfw_monitors[i]);
    }
    return monitors;
}

int mve::Monitor::refresh_rate() const
{
    const GLFWvidmode* mode = glfwGetVideoMode(m_monitor);
    return mode->refreshRate;
}
std::string mve::Monitor::name() const
{
    return glfwGetMonitorName(m_monitor);
}
