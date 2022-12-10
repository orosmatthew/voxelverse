#include "app.hpp"

#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "logger.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "uniform_struct_layout.hpp"
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

        mve::UniformStructLayout uniform_struct("UniformBufferObject");
        uniform_struct.push_back("model", mve::UniformType::e_mat4);
        uniform_struct.push_back("view", mve::UniformType::e_mat4);
        uniform_struct.push_back("proj", mve::UniformType::e_mat4);

        mve::UniformLocation model_location = uniform_struct.get_location("model");
        mve::UniformLocation view_location = uniform_struct.get_location("view");
        mve::UniformLocation proj_location = uniform_struct.get_location("proj");

        mve::Renderer::UniformBufferHandle uniform_handle = renderer.create_uniform_buffer(uniform_struct);

        std::chrono::high_resolution_clock ::time_point begin_time = std::chrono::high_resolution_clock::now();
        int frame_count = 0;

        auto start_time = std::chrono::high_resolution_clock::now();

        while (!window.should_close()) {
            window.update();

            if (window.is_key_pressed(mve::InputKey::e_escape)) {
                break;
            }

            if (!is_triangle_removed && window.is_key_pressed(mve::InputKey::e_t)) {
                renderer.queue_destroy(vertex_data_handle3);
                is_triangle_removed = true;
            }

            auto current_time = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

            glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 view
                = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 proj = glm::perspective(
                glm::radians(45.0f), (float)renderer.get_extent().x / (float)renderer.get_extent().y, 0.1f, 10.0f);
            proj[1][1] *= -1;

            renderer.begin(window);

            renderer.update_uniform(uniform_handle, model_location, model);
            renderer.update_uniform(uniform_handle, view_location, view);
            renderer.update_uniform(uniform_handle, proj_location, proj);

            renderer.bind(uniform_handle);

            renderer.bind(vertex_data_handle_indexed);

            renderer.draw(index_buffer_handle);

            renderer.end(window);

            std::chrono::high_resolution_clock ::time_point end_time = std::chrono::high_resolution_clock ::now();

            if (std::chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time).count() >= 1000000) {
                begin_time = std::chrono::high_resolution_clock ::now();
                LOG->info("Framerate: {}", frame_count);
                frame_count = 0;
            }

            frame_count++;
        }
    }
}
