#include "app.hpp"

#include <chrono>

#include <glm/ext.hpp>

#include "logger.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "uniform_struct_layout.hpp"
#include "window.hpp"

namespace app {

void run()
{
    LOG->debug("Creating window");

    mve::Window window("Mini Vulkan Engine", glm::ivec2(800, 600));

    window.set_min_size({ 800, 600 });

    mve::Renderer renderer(window, "Vulkan Testing", 0, 0, 1);

    mve::Shader vertex_shader("../res/bin/shader/simple.vert.spv", mve::ShaderType::vertex);
    mve::Shader fragment_shader("../res/bin/shader/simple.frag.spv", mve::ShaderType::fragment);

    mve::VertexLayout vertex_layout {};
    vertex_layout.push_back(mve::VertexAttributeType::vec2); // 2D position
    vertex_layout.push_back(mve::VertexAttributeType::vec3); // Color

    mve::VertexData place_data(vertex_layout);
    place_data.push_back({ -0.5f, -0.5f });
    place_data.push_back({ 1.0f, 0.0f, 0.0f });
    place_data.push_back({ 0.5f, -0.5f });
    place_data.push_back({ 0.0f, 1.0f, 0.0f });
    place_data.push_back({ 0.5f, 0.5f });
    place_data.push_back({ 0.0f, 0.0f, 1.0f });
    place_data.push_back({ -0.5f, 0.5f });
    place_data.push_back({ 1.0f, 1.0f, 1.0f });

    const std::vector<uint32_t> plane_indices = { 0, 1, 2, 2, 3, 0 };

    mve::VertexBufferHandle vertex_buffer = renderer.create_vertex_buffer(place_data);
    mve::IndexBufferHandle index_buffer = renderer.create_index_buffer(plane_indices);

    mve::DescriptorSetLayoutHandle descriptor_set_layout_handle
        = renderer.create_descriptor_set_layout({ mve::DescriptorType::e_uniform_buffer });

    mve::GraphicsPipelineLayoutHandle graphics_pipeline_layout
        = renderer.create_graphics_pipeline_layout({ descriptor_set_layout_handle });

    mve::GraphicsPipelineHandle graphics_pipeline
        = renderer.create_graphics_pipeline(graphics_pipeline_layout, vertex_shader, fragment_shader, vertex_layout);

    mve::DescriptorSetHandle descriptor_set_handle = renderer.create_descriptor_set(descriptor_set_layout_handle);

    mve::UniformStructLayout uniform_struct("UniformBufferObject");
    uniform_struct.push_back("model", mve::UniformType::mat4);
    uniform_struct.push_back("view", mve::UniformType::mat4);
    uniform_struct.push_back("proj", mve::UniformType::mat4);

    mve::UniformLocation model_location = uniform_struct.location_of("model");
    mve::UniformLocation view_location = uniform_struct.location_of("view");
    mve::UniformLocation proj_location = uniform_struct.location_of("proj");

    mve::UniformBufferHandle uniform_handle = renderer.create_uniform_buffer(uniform_struct, descriptor_set_handle, 0);

    std::chrono::high_resolution_clock::time_point begin_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    renderer.update_uniform(uniform_handle, view_location, view);

    auto resize_func = [&](glm::ivec2 new_size) {
        renderer.resize(window);
        glm::mat4 proj = glm::perspective(
            glm::radians(45.0f), (float)renderer.extent().x / (float)renderer.extent().y, 0.1f, 10.0f);
        proj[1][1] *= -1;
        renderer.update_uniform(uniform_handle, proj_location, proj);
    };

    window.set_resize_callback(resize_func);

    std::invoke(resize_func, window.size());

    std::vector<mve::Monitor> monitors = mve::Monitor::list();

    while (!window.should_close()) {
        window.poll_events();

        if (window.is_key_pressed(mve::InputKey::escape)) {
            break;
        }

        if (window.is_key_pressed(mve::InputKey::f)) {
            if (!window.is_fullscreen()) {
                window.fullscreen(true);
            }
            else {
                window.windowed();
            }
        }

        auto current_time = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        renderer.update_uniform(uniform_handle, model_location, model, false);

        renderer.begin(window);

        renderer.bind_graphics_pipeline(graphics_pipeline);

        renderer.bind_descriptor_set(descriptor_set_handle, graphics_pipeline_layout);

        renderer.bind_vertex_buffer(vertex_buffer);

        renderer.draw_index_buffer(index_buffer);

        renderer.end(window);

        std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time).count() >= 1000000) {
            begin_time = std::chrono::high_resolution_clock::now();
            LOG->info("Framerate: {}", frame_count);
            frame_count = 0;
        }

        frame_count++;
    }
}
}
