#include "window.hpp"
#include "logger.hpp"

namespace mve {

    Window::Window(const std::string &title, glm::ivec2 size)
        : m_was_resized(false)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        auto window_deleter = [](GLFWwindow *window) {
            glfwDestroyWindow(window);
            glfwTerminate();
        };

        m_glfw_window
            = UniqueGlfwWindow(glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr), window_deleter);

        glfwSetWindowUserPointer(m_glfw_window.get(), this);
        glfwSetFramebufferSizeCallback(m_glfw_window.get(), glfw_framebuffer_resize_callback);
    }

    GLFWwindow *Window::get_glfw_handle() const
    {
        return m_glfw_window.get();
    }

    void Window::glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height)
    {
        auto *instance = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        instance->m_was_resized = true;
        if (instance->m_resize_callback.has_value()) {
            (*(instance->m_resize_callback))(glm::ivec2(width, height));
        }
    }

    bool Window::was_resized() const
    {
        return m_was_resized;
    }

    glm::ivec2 Window::get_size() const
    {
        int width;
        int height;
        glfwGetFramebufferSize(m_glfw_window.get(), &width, &height);
        return { width, height };
    }

    bool Window::should_close() const
    {
        return glfwWindowShouldClose(m_glfw_window.get());
    }

    void Window::update()
    {
        m_was_resized = false;
        glfwPollEvents();
    }

    void Window::wait_for_events() const
    {
        glfwWaitEvents();
    }

    void Window::set_resize_callback(const std::function<void(glm::ivec2)> &resize_callback)
    {
        m_resize_callback = resize_callback;
    }

    void Window::remove_resize_callback()
    {
        m_resize_callback.reset();
    }

    InputState Window::get_key(InputKey key)
    {
        return static_cast<InputState>(glfwGetKey(m_glfw_window.get(), static_cast<int>(key)));
    }

    glm::vec2 Window::get_cursor_pos(bool clamped_to_window)
    {
        glm::dvec2 mouse_pos;
        glfwGetCursorPos(m_glfw_window.get(), &(mouse_pos.x), &(mouse_pos.y));
        if (clamped_to_window) {
            glm::ivec2 window_size = get_size();
            mouse_pos.x = glm::clamp(mouse_pos.x, 0.0, static_cast<double>(window_size.x));
            mouse_pos.y = glm::clamp(mouse_pos.y, 0.0, static_cast<double>(window_size.y));
        }
        return { mouse_pos };
    }
}