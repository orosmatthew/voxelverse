#include "app.hpp"

#include "logger.hpp"

#include "renderer.hpp"

#include "window.hpp"

namespace app {

    void run()
    {
        LOG->debug("Creating window");

        auto window = mve::Window("Mini Vulkan Engine", glm::ivec2(800, 600));

        auto vertex_shader = mve::Shader("../res/bin/shader/simple.vert.spv");
        auto fragment_shader = mve::Shader("../res/bin/shader/simple.frag.spv");

        auto vertex_layout = mve::VertexLayout();
        vertex_layout.push_back(mve::VertexAttributeType::e_vec2); // 2D position
        vertex_layout.push_back(mve::VertexAttributeType::e_vec3); // Color

        auto renderer = mve::Renderer(window, "Vulkan Testing", 0, 0, 1, vertex_shader, fragment_shader, vertex_layout);

        window.set_resize_callback([&](glm::ivec2 new_size) { renderer.draw_frame(window); });

        auto big_triangle = mve::VertexData(vertex_layout);
        big_triangle.add_data({ 0.0f, -0.5f });
        big_triangle.add_data({ 1.0f, 0.0f, 0.0f });
        big_triangle.add_data({ 0.5f, 0.5f });
        big_triangle.add_data({ 0.0f, 1.0f, 0.0f });
        big_triangle.add_data({ -0.5f, 0.5f });
        big_triangle.add_data({ 0.0f, 0.0f, 1.0f });

        auto bottom_right_triangle = mve::VertexData(vertex_layout);
        bottom_right_triangle.add_data({ 0.0f + 0.7f, -0.1f + 0.7f });
        bottom_right_triangle.add_data({ 1.0f, 0.0f, 0.0f });
        bottom_right_triangle.add_data({ 0.1f + 0.7f, 0.1f + 0.7f });
        bottom_right_triangle.add_data({ 0.0f, 1.0f, 0.0f });
        bottom_right_triangle.add_data({ -0.1f + 0.7f, 0.1f + 0.7f });
        bottom_right_triangle.add_data({ 0.0f, 0.0f, 1.0f });

        auto top_left_triangle = mve::VertexData(vertex_layout);
        top_left_triangle.add_data({ 0.0f - 0.7f, -0.1f - 0.7f });
        top_left_triangle.add_data({ 1.0f, 0.0f, 0.0f });
        top_left_triangle.add_data({ 0.1f - 0.7f, 0.1f - 0.7f });
        top_left_triangle.add_data({ 0.0f, 1.0f, 0.0f });
        top_left_triangle.add_data({ -0.1f - 0.7f, 0.1f - 0.7f });
        top_left_triangle.add_data({ 0.0f, 0.0f, 1.0f });

        mve::Renderer::VertexDataHandle vertex_data_handle1 = renderer.upload_vertex_data(big_triangle);
        mve::Renderer::VertexDataHandle vertex_data_handle2 = renderer.upload_vertex_data(bottom_right_triangle);
        mve::Renderer::VertexDataHandle vertex_data_handle3 = renderer.upload_vertex_data(top_left_triangle);

        while (!window.should_close()) {
            window.update();

            if (window.get_key(mve::InputKey::e_escape)) {
                break;
            }

            renderer.draw_frame(window);
        }
    }
}
