#include "app.hpp"

#include "logger.hpp"

#include "renderer.hpp"

#include "window.hpp"

using namespace mve;

namespace app {

    void run()
    {
        LOG->debug("Creating window");

        auto window = Window("Mini Vulkan Engine", glm::ivec2(800, 600));

        auto vertex_shader = Shader("shaders/simple.vert", ShaderType::e_vertex, true);
        auto fragment_shader = Shader("shaders/simple.frag", ShaderType::e_fragment, true);

        auto renderer = Renderer(window, "Vulkan Testing", 0, 0, 1, vertex_shader, fragment_shader);

        window.set_resize_callback([&](glm::ivec2 new_size) { renderer.draw_frame(window); });

        while (!window.should_close()) {
            window.update();

            if (window.get_key(InputKey::e_escape)) {
                break;
            }

            renderer.draw_frame(window);
        }
    }
}
