#include "window.hpp"

#include <stdexcept>

#include <glm/common.hpp>

#include "logger.hpp"

namespace mve {

Window::Window(const std::string& title, glm::ivec2 size, bool resizable)
    : m_resizable(resizable)
    , m_fullscreen(false)
    , m_hidden(false)
    , m_minimized(false)
    , m_maximized(false)
    , m_focused(true)
    , m_min_size({ 0, 0 })
    , m_cursor_hidden(false)
    , m_cursor_in_window(true)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    auto window_deleter = [](GLFWwindow* window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    };

    m_glfw_window = UniqueGlfwWindow(glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr), window_deleter);
    glfwWindowHint(GLFW_AUTO_ICONIFY, 0);

    glfwSetWindowUserPointer(m_glfw_window.get(), this);
    glfwSetFramebufferSizeCallback(m_glfw_window.get(), glfw_framebuffer_resize_callback);
    glfwSetWindowIconifyCallback(m_glfw_window.get(), glfw_iconify_callback);
    glfwSetWindowFocusCallback(m_glfw_window.get(), glfw_focused_callback);
    glfwGetFramebufferSize(m_glfw_window.get(), &(m_size.x), &(m_size.y));
    glfwSetCursorEnterCallback(m_glfw_window.get(), glfw_cursor_enter_callback);
    glfwSetKeyCallback(m_glfw_window.get(), glfw_key_callback);
}

GLFWwindow* Window::glfw_handle() const
{
    return m_glfw_window.get();
}

void Window::glfw_framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
    auto* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    glfwGetFramebufferSize(window, &(instance->m_size.x), &(instance->m_size.y));

    if (instance->m_resize_callback.has_value()) {
        (*(instance->m_resize_callback))(glm::ivec2(width, height));
    }
}

glm::ivec2 Window::size() const
{
    return m_size;
}

bool Window::should_close() const
{
    return glfwWindowShouldClose(m_glfw_window.get());
}

void Window::poll_events()
{
    glfwPollEvents();
    m_keys_pressed.clear();
    for (InputKey key : m_current_keys_down) {
        if (!m_keys_down.contains(key)) {
            m_keys_pressed.insert(key);
        }
    }
    m_keys_released = m_current_keys_released;
    m_current_keys_released.clear();
    m_keys_down = m_current_keys_down;
}

void Window::wait_for_events() const
{
    glfwWaitEvents();
}

void Window::set_resize_callback(const std::function<void(glm::ivec2)>& resize_callback)
{
    m_resize_callback = resize_callback;
}

void Window::remove_resize_callback()
{
    m_resize_callback.reset();
}

glm::vec2 Window::get_cursor_pos(bool clamped_to_window)
{
    glm::dvec2 mouse_pos;
    glfwGetCursorPos(m_glfw_window.get(), &(mouse_pos.x), &(mouse_pos.y));
    if (clamped_to_window) {
        glm::ivec2 window_size = size();
        mouse_pos.x = glm::clamp(mouse_pos.x, 0.0, static_cast<double>(window_size.x));
        mouse_pos.y = glm::clamp(mouse_pos.y, 0.0, static_cast<double>(window_size.y));
    }
    return { mouse_pos };
}

void Window::toggle_fullscreen()
{
    if (!m_fullscreen) {
        glfwGetWindowPos(m_glfw_window.get(), &(m_pos.x), &(m_pos.y));

        Monitor monitor = current_monitor();

        m_fullscreen = true;
        glm::ivec2 size = monitor.size();
        glfwSetWindowMonitor(m_glfw_window.get(), monitor.glfw_handle(), 0, 0, size.x, size.y, GLFW_DONT_CARE);
    }
    else {
        m_fullscreen = false;
        glfwSetWindowMonitor(m_glfw_window.get(), nullptr, m_pos.x, m_pos.y, m_size.x, m_size.y, GLFW_DONT_CARE);
    }
}

Monitor Window::current_monitor() const
{
    int monitor_count;
    GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
    GLFWmonitor* monitor;

    if (monitor_count == 1) {
        return 0;
    }

    if (is_fullscreen()) {
        monitor = glfwGetWindowMonitor(m_glfw_window.get());
        for (int i = 0; i < monitor_count; i++) {
            if (monitors[i] == monitor) {
                return Monitor(monitor);
            }
        }
        return 0;
    }
    else {
        glm::ivec2 pos;
        glfwGetWindowPos(m_glfw_window.get(), &(pos.x), &(pos.y));

        for (int i = 0; i < monitor_count; i++) {
            glm::ivec2 workarea_pos;
            glm::ivec2 workarea_size;

            monitor = monitors[i];
            glfwGetMonitorWorkarea(
                monitor, &(workarea_pos.x), &(workarea_pos.y), &(workarea_size.x), &(workarea_size.y));
            if (pos.x >= workarea_pos.x && pos.x <= (workarea_pos.x + workarea_size.x) && pos.y >= workarea_pos.y
                && pos.y <= (workarea_pos.y + workarea_size.y)) {
                return Monitor(monitor);
            }
        }
    }
    return 0;
}

bool Window::is_fullscreen() const
{
    return m_fullscreen;
}

void Window::hide()
{
    m_hidden = true;
    glfwHideWindow(m_glfw_window.get());
}

void Window::show()
{
    m_hidden = false;
    glfwShowWindow(m_glfw_window.get());
}

bool Window::is_hidden() const
{
    return m_hidden;
}

void Window::glfw_iconify_callback(GLFWwindow* window, int iconified)
{
    LOG->info("Here!: {}", iconified);
    auto* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    if (iconified) {
        instance->m_minimized = true;
    }
    else {
        instance->m_minimized = false;
    }
}

bool Window::is_minimized() const
{
    return m_minimized;
}

void Window::maximize()
{
    if (m_resizable) {
        m_maximized = true;
        glfwMaximizeWindow(m_glfw_window.get());
    }
}

bool Window::is_maximized() const
{
    return m_maximized;
}

bool Window::is_focused() const
{
    return m_focused;
}

void Window::glfw_focused_callback(GLFWwindow* window, int focused)
{
    auto* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    instance->m_focused = static_cast<bool>(focused);
}

void Window::minimize()
{
    glfwIconifyWindow(m_glfw_window.get());
}

void Window::restore()
{
    if (m_resizable) {
        glfwRestoreWindow(m_glfw_window.get());
    }
}

void Window::set_title(const std::string& title)
{
    glfwSetWindowTitle(m_glfw_window.get(), title.c_str());
}

void Window::move_to(glm::ivec2 pos)
{
    glfwSetWindowPos(m_glfw_window.get(), pos.x, pos.y);
}

void Window::fullscreen_to(Monitor monitor)
{
    const GLFWvidmode* mode = glfwGetVideoMode(monitor.glfw_handle());
    glfwSetWindowMonitor(
        m_glfw_window.get(), monitor.glfw_handle(), 0, 0, mode->width, mode->height, mode->refreshRate);
}

void Window::set_min_size(glm::ivec2 size)
{
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowSizeLimits(m_glfw_window.get(), size.x, size.y, mode->width, mode->height);
}

void Window::resize(glm::ivec2 size)
{
    glfwSetWindowSize(m_glfw_window.get(), size.x, size.y);
}

glm::ivec2 Window::position() const
{
    return m_pos;
}

void Window::set_clipboard_text(const std::string& text)
{
    glfwSetClipboardString(m_glfw_window.get(), text.c_str());
}

std::string Window::clipboard_text() const
{
    return glfwGetClipboardString(m_glfw_window.get());
}

void Window::show_cursor()
{
    glfwSetInputMode(m_glfw_window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_cursor_hidden = false;
}

void Window::hide_cursor()
{
    glfwSetInputMode(m_glfw_window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    m_cursor_hidden = true;
}

bool Window::is_cursor_hidden() const
{
    return m_cursor_hidden;
}

void Window::enable_cursor()
{
    glfwSetInputMode(m_glfw_window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_cursor_hidden = false;
}

void Window::disable_cursor()
{
    glfwSetInputMode(m_glfw_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    m_cursor_hidden = true;
}

bool Window::is_cursor_in_window() const
{
    return m_cursor_in_window;
}

void Window::glfw_cursor_enter_callback(GLFWwindow* window, int entered)
{
    auto* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    instance->m_cursor_in_window = static_cast<bool>(entered);
}
bool Window::is_key_pressed(InputKey key)
{
    return m_keys_pressed.contains(key);
}

void Window::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        instance->m_current_keys_down.insert(static_cast<InputKey>(key));
    }
    else if (action == GLFW_RELEASE) {
        instance->m_current_keys_down.erase(static_cast<InputKey>(key));
        instance->m_current_keys_released.insert(static_cast<InputKey>(key));
    }
}

bool Window::is_key_down(InputKey key) const
{
    return m_keys_down.contains(key);
}

bool Window::is_key_released(InputKey key) const
{
    return m_keys_released.contains(key);
}

}