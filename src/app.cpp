#include "app.hpp"

#include "logger.hpp"

#include "renderer.hpp"

#include "window.hpp"

namespace app {

    void run()
    {
        LOG->debug("Creating window");

        auto window = mve::Window("Mini Vulkan Engine", glm::ivec2(800, 600));

        auto vertex_shader = mve::Shader("shaders/simple.vert", mve::ShaderType::e_vertex, true);
        auto fragment_shader = mve::Shader("shaders/simple.frag", mve::ShaderType::e_fragment, true);

        auto renderer = mve::Renderer(window, "Vulkan Testing", 0, 0, 1, vertex_shader, fragment_shader);

        window.set_resize_callback([&](glm::ivec2 new_size) { renderer.draw_frame(window); });

        while (!window.should_close()) {
            window.update();
            renderer.draw_frame(window);
        }
    }
}
