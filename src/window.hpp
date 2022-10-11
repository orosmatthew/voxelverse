#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <glm/glm.hpp>
#include <optional>

namespace mve {
    class Window {
    public:
        Window(const std::string &title, glm::ivec2 size);

        [[nodiscard]] GLFWwindow *get_glfw_handle() const;

        [[nodiscard]] bool was_resized() const;

        [[nodiscard]] glm::ivec2 get_size() const;

        [[nodiscard]] bool should_close() const;

        void set_resize_callback(const std::function<void(glm::ivec2)>& resize_callback);

        void remove_resize_callback();

        void update();

        void wait_for_events() const;

    private:
        typedef std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow *)>> UniqueGlfwWindow;

        UniqueGlfwWindow m_glfw_window;
        bool m_was_resized;
        std::optional<std::function<void(glm::ivec2)>> m_resize_callback;

        static void glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height);
    };
}