#include "window.hpp"

#include <stdexcept>

#include "../logger.hpp"
#include "common.hpp"
#include "math/math.hpp"

namespace mve {

Window::Window(const std::string& title, mve::Vector2i size, bool resizable)
    : m_resizable(resizable)
    , m_fullscreen(false)
    , m_hidden(false)
    , m_minimized(false)
    , m_maximized(false)
    , m_focused(true)
    , m_min_size(0, 0)
    , m_cursor_hidden(false)
    , m_cursor_in_window(true)
    , m_event_waiting(false)
    , m_mouse_delta(0.0f)
    , m_mouse_pos_prev(0.0f)
    , m_current_mouse_pos(0.0f)
    , m_mouse_pos(0.0f)
{
    m_windowed_size = m_size;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, m_resizable);

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
    glfwSetMouseButtonCallback(m_glfw_window.get(), glfw_mouse_button_callback);
    glfwSetCursorPosCallback(m_glfw_window.get(), glfw_cursor_pos_callback);
    glfwSetScrollCallback(m_glfw_window.get(), glfw_scroll_callback);
    glfwSetKeyCallback(m_glfw_window.get(), glfw_key_callback);
    glfwSetCharCallback(m_glfw_window.get(), glfw_char_callback);
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
        (*(instance->m_resize_callback))(mve::Vector2i(width, height));
    }
}

mve::Vector2i Window::size() const
{
    return m_size;
}

bool Window::should_close() const
{
    return glfwWindowShouldClose(m_glfw_window.get());
}

void Window::poll_events()
{
    if (m_event_waiting) {
        glfwWaitEvents();
    }
    else {
        glfwPollEvents();
    }

    m_mouse_pos = m_current_mouse_pos;
    m_mouse_delta = m_mouse_pos - m_mouse_pos_prev;
    m_mouse_pos_prev = m_mouse_pos;

    m_keys_pressed.clear();
    for (Key key : m_current_keys_down) {
        if (!m_keys_down.contains(key)) {
            m_keys_pressed.insert(key);
        }
    }
    m_keys_released = m_current_keys_released;
    m_current_keys_released.clear();
    m_keys_down = m_current_keys_down;

    m_mouse_buttons_pressed.clear();
    for (MouseButton button : m_current_mouse_buttons_down) {
        if (!m_mouse_buttons_down.contains(button)) {
            m_mouse_buttons_pressed.insert(button);
        }
    }
    m_mouse_buttons_released = m_current_mouse_buttons_released;
    m_current_mouse_buttons_released.clear();
    m_mouse_buttons_down = m_current_mouse_buttons_down;

    m_scroll_offset = m_current_scroll_offset;
    m_current_scroll_offset = mve::Vector2(0);

    std::swap(m_current_input_stream, m_input_stream);
    m_current_input_stream.clear();

    std::swap(m_current_keys_repeated, m_keys_repeated);
    m_current_keys_repeated.clear();
}

mve::Vector2 Window::mouse_scroll() const
{
    return m_scroll_offset;
}

void Window::wait_for_events() const
{
    glfwWaitEvents();
}

void Window::set_resize_callback(const std::function<void(mve::Vector2i)>& resize_callback)
{
    m_resize_callback = resize_callback;
}

void Window::remove_resize_callback()
{
    m_resize_callback.reset();
}

mve::Vector2 Window::get_cursor_pos(bool clamped_to_window)
{
    double glfw_cursor_pos[2];
    glfwGetCursorPos(m_glfw_window.get(), &(glfw_cursor_pos[0]), &(glfw_cursor_pos[1]));
    mve::Vector2 mouse_pos_val;
    if (clamped_to_window) {
        mve::Vector2i window_size = size();
        mouse_pos_val.x = mve::clamp(static_cast<float>(glfw_cursor_pos[0]), 0.0f, static_cast<float>(window_size.x));
        mouse_pos_val.y = mve::clamp(static_cast<float>(glfw_cursor_pos[1]), 0.0f, static_cast<float>(window_size.y));
    }
    return { mouse_pos_val };
}

Monitor Window::current_monitor() const
{
    int monitor_count;
    GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

    if (m_fullscreen) {
        return Monitor(glfwGetWindowMonitor(m_glfw_window.get()));
    }
    else {
        mve::Vector2i pos;
        glfwGetWindowPos(m_glfw_window.get(), &(pos.x), &(pos.y));

        for (int i = 0; i < monitor_count; i++) {
            mve::Vector2i workarea_pos;
            mve::Vector2i workarea_size;

            GLFWmonitor* monitor = monitors[i];
            glfwGetMonitorWorkarea(
                monitor, &(workarea_pos.x), &(workarea_pos.y), &(workarea_size.x), &(workarea_size.y));
            if (pos.x >= workarea_pos.x && pos.x <= (workarea_pos.x + workarea_size.x) && pos.y >= workarea_pos.y
                && pos.y <= (workarea_pos.y + workarea_size.y)) {
                return Monitor(monitor);
            }
        }
    }
    MVE_ASSERT(false, "[Window] Failed to get current monitor")
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

void Window::move_to(mve::Vector2i pos)
{
    glfwSetWindowPos(m_glfw_window.get(), pos.x, pos.y);
}

void Window::fullscreen_to(Monitor monitor, bool use_native)
{
    mve::Vector2i monitor_size;
    if (m_resizable && use_native) {
        monitor_size = monitor.size();
    }
    else {
        monitor_size = m_size;
    }
    glfwSetWindowMonitor(
        m_glfw_window.get(), monitor.glfw_handle(), 0, 0, monitor_size.x, monitor_size.y, GLFW_DONT_CARE);
}

void Window::set_min_size(mve::Vector2i size)
{
    if (size.x > m_size.x || size.y > m_size.y) {
        resize(size);
    }
    glfwSetWindowSizeLimits(m_glfw_window.get(), size.x, size.y, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void Window::resize(mve::Vector2i size)
{
    glfwSetWindowSize(m_glfw_window.get(), size.x, size.y);
}

mve::Vector2i Window::position() const
{
    if (!m_fullscreen) {
        return m_pos;
    }
    else {
        return { 0, 0 };
    }
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
bool Window::is_key_pressed(Key key) const
{
    return m_keys_pressed.contains(key);
}

void Window::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    switch (action) {
    case GLFW_PRESS:
        instance->m_current_keys_down.insert(static_cast<Key>(key));
        break;
    case GLFW_REPEAT:
        instance->m_current_keys_repeated.insert(static_cast<Key>(key));
        break;
    case GLFW_RELEASE:
        instance->m_current_keys_down.erase(static_cast<Key>(key));
        instance->m_current_keys_released.insert(static_cast<Key>(key));
        break;
    }
}

bool Window::is_key_down(Key key) const
{
    return m_keys_down.contains(key);
}

bool Window::is_key_released(Key key) const
{
    return m_keys_released.contains(key);
}
void Window::enable_event_waiting()
{
    m_event_waiting = true;
}
void Window::disable_event_waiting()
{
    m_event_waiting = false;
}

bool Window::is_resizable() const
{
    return m_resizable;
}

void Window::windowed()
{
    if (m_fullscreen) {
        m_fullscreen = false;
        glfwSetWindowMonitor(
            m_glfw_window.get(), nullptr, m_pos.x, m_pos.y, m_windowed_size.x, m_windowed_size.y, GLFW_DONT_CARE);
    }
}

void Window::fullscreen(bool use_native)
{
    mve::Vector2i monitor_size;
    if (!m_fullscreen) {
        m_windowed_size = m_size;
        glfwGetWindowPos(m_glfw_window.get(), &(m_pos.x), &(m_pos.y));
        Monitor monitor = current_monitor();
        if (m_resizable && use_native) {
            monitor_size = monitor.size();
        }
        else {
            monitor_size = m_size;
        }
        glfwSetWindowMonitor(
            m_glfw_window.get(), monitor.glfw_handle(), 0, 0, monitor_size.x, monitor_size.y, GLFW_DONT_CARE);
        m_fullscreen = true;
    }
    else if (m_resizable && use_native && m_size != current_monitor().size()) {
        Monitor monitor = current_monitor();
        monitor_size = monitor.size();
        glfwSetWindowMonitor(
            m_glfw_window.get(), monitor.glfw_handle(), 0, 0, monitor_size.x, monitor_size.y, GLFW_DONT_CARE);
    }
}

void Window::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    auto* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        instance->m_current_mouse_buttons_down.insert(static_cast<MouseButton>(button));
    }
    else if (action == GLFW_RELEASE) {
        instance->m_current_mouse_buttons_down.erase(static_cast<MouseButton>(button));
        instance->m_current_mouse_buttons_released.insert(static_cast<MouseButton>(button));
    }
}

void Window::glfw_cursor_pos_callback(GLFWwindow* window, double pos_x, double pos_y)
{
    auto* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    instance->m_current_mouse_pos = mve::Vector2(static_cast<float>(pos_x), static_cast<float>(pos_y));
}
bool Window::is_mouse_button_down(MouseButton button) const
{
    return m_mouse_buttons_down.contains(button);
}
bool Window::is_mouse_button_pressed(MouseButton button) const
{
    return m_mouse_buttons_pressed.contains(button);
}
bool Window::is_mouse_button_released(MouseButton button) const
{
    return m_mouse_buttons_released.contains(button);
}
mve::Vector2 Window::mouse_pos() const
{
    return m_mouse_pos;
}
mve::Vector2 Window::mouse_delta() const
{
    return m_mouse_delta;
}
void Window::glfw_scroll_callback(GLFWwindow* window, double offset_x, double offset_y)
{
    auto* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    instance->m_current_scroll_offset.x += static_cast<float>(offset_x);
    instance->m_current_scroll_offset.y += static_cast<float>(offset_y);
}
void Window::set_cursor_pos(mve::Vector2 pos)
{
    glfwSetCursorPos(m_glfw_window.get(), static_cast<float>(pos.x), static_cast<float>(pos.y));
}
void Window::glfw_char_callback(GLFWwindow* window, unsigned int codepoint)
{
    auto* instance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    wchar_t wchar = static_cast<wchar_t>(codepoint);
    std::wstring wstr;
    wstr.push_back(wchar);
    std::string str(wstr.begin(), wstr.end());
    instance->m_current_input_stream.push_back(str);
}
bool Window::is_key_repeated(Key key) const
{
    return m_keys_repeated.contains(key);
}
}
