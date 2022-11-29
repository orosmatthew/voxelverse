#include "app.hpp"

#include <chrono>

#include "logger.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "window.hpp"

namespace app {

    void run()
    {
        LOG->debug("Creating window");

        auto window = mve::Window("Mini Vulkan Engine", glm::ivec2(800, 600));

        auto vertex_shader = mve::Shader("../res/bin/shader/simple.vert.spv", mve::ShaderType::e_vertex);
        auto fragment_shader = mve::Shader("../res/bin/shader/simple.frag.spv", mve::ShaderType::e_fragment);

        auto vertex_layout = mve::VertexLayout();
        vertex_layout.push_back(mve::VertexAttributeType::e_vec2); // 2D position
        vertex_layout.push_back(mve::VertexAttributeType::e_vec3); // Color

        auto renderer = mve::Renderer(window, "Vulkan Testing", 0, 0, 1, vertex_shader, fragment_shader, vertex_layout);

        // window.set_resize_callback([&](glm::ivec2 new_size) { renderer.draw_frame(window); });

        auto big_triangle = mve::VertexData(vertex_layout);
        big_triangle.push_back({ 0.0f, -0.5f });
        big_triangle.push_back({ 1.0f, 0.0f, 0.0f });
        big_triangle.push_back({ 0.5f, 0.5f });
        big_triangle.push_back({ 0.0f, 1.0f, 0.0f });
        big_triangle.push_back({ -0.5f, 0.5f });
        big_triangle.push_back({ 0.0f, 0.0f, 1.0f });

        auto bottom_right_triangle = mve::VertexData(vertex_layout);
        bottom_right_triangle.push_back({ 0.0f + 0.3f, -0.1f + 0.3f });
        bottom_right_triangle.push_back({ 1.0f, 0.0f, 0.0f });
        bottom_right_triangle.push_back({ 0.1f + 0.3f, 0.1f + 0.3f });
        bottom_right_triangle.push_back({ 0.0f, 1.0f, 0.0f });
        bottom_right_triangle.push_back({ -0.1f + 0.3f, 0.1f + 0.3f });
        bottom_right_triangle.push_back({ 0.0f, 0.0f, 1.0f });

        auto top_left_triangle = mve::VertexData(vertex_layout);
        top_left_triangle.push_back({ 0.0f - 0.7f, -0.1f - 0.7f });
        top_left_triangle.push_back({ 1.0f, 0.0f, 0.0f });
        top_left_triangle.push_back({ 0.1f - 0.7f, 0.1f - 0.7f });
        top_left_triangle.push_back({ 0.0f, 1.0f, 0.0f });
        top_left_triangle.push_back({ -0.1f - 0.7f, 0.1f - 0.7f });
        top_left_triangle.push_back({ 0.0f, 0.0f, 1.0f });

        auto index_triangle = mve::VertexData(vertex_layout);
        index_triangle.push_back({ -0.5f, -0.5f });
        index_triangle.push_back({ 1.0f, 0.0f, 0.0f });
        index_triangle.push_back({ 0.5f, -0.5f });
        index_triangle.push_back({ 0.0f, 1.0f, 0.0f });
        index_triangle.push_back({ 0.5f, 0.5f });
        index_triangle.push_back({ 0.0f, 0.0f, 1.0f });
        index_triangle.push_back({ -0.5f, 0.5f });
        index_triangle.push_back({ 1.0f, 1.0f, 1.0f });

        const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

        mve::Renderer::VertexBufferHandle vertex_data_handle1 = renderer.upload(big_triangle);
        mve::Renderer::VertexBufferHandle vertex_data_handle2 = renderer.upload(bottom_right_triangle);
        mve::Renderer::VertexBufferHandle vertex_data_handle3 = renderer.upload(top_left_triangle);

        mve::Renderer::VertexBufferHandle vertex_data_handle_indexed = renderer.upload(index_triangle);
        mve::Renderer::IndexBufferHandle index_buffer_handle = renderer.upload(indices);

        bool is_triangle_removed = false;

        std::chrono::steady_clock::time_point begin_time = std::chrono::steady_clock::now();
        int frame_count = 0;
        while (!window.should_close()) {
            window.update();

            if (window.is_key_pressed(mve::InputKey::e_escape)) {
                break;
            }

            if (!is_triangle_removed && window.is_key_pressed(mve::InputKey::e_t)) {
                renderer.queue_destroy(vertex_data_handle3);
                is_triangle_removed = true;
            }

            renderer.begin(window);

            renderer.bind(vertex_data_handle_indexed);

            renderer.update_uniforms();

            renderer.draw(index_buffer_handle);

            //
            //            renderer.draw(vertex_data_handle1);
            //            renderer.draw(vertex_data_handle2);
            //
            //            if (!is_triangle_removed) {
            //                renderer.draw(vertex_data_handle3);
            //            }

            renderer.end(window);

            std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();

            if (std::chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time).count() >= 1000000) {
                begin_time = std::chrono::steady_clock::now();
                LOG->info("Framerate: {}", frame_count);
                frame_count = 0;
            }

            frame_count++;
        }
    }
}
