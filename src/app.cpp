#include "app.hpp"

#include <chrono>

#include <glm/ext.hpp>

#include "logger.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "uniform_struct_layout.hpp"
#include "util.hpp"
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

    mve::ModelData model_data = mve::load_model("../res/viking_room.obj");

    mve::VertexBufferHandle model_vertex_buffer = renderer.create_vertex_buffer(model_data.vertex_data);
    mve::IndexBufferHandle model_index_buffer = renderer.create_index_buffer(model_data.indices);

    mve::GraphicsPipelineHandle graphics_pipeline
        = renderer.create_graphics_pipeline(vertex_shader, fragment_shader, model_data.vertex_data.layout());

    mve::DescriptorSetHandle descriptor_set_handle = renderer.create_descriptor_set(graphics_pipeline, 0);

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

    mve::TextureHandle texture = renderer.create_texture("../res/viking_room.png", descriptor_set_handle, 1);

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

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

        if (window.is_key_down(mve::InputKey::left)) {
            model = glm::rotate(model, glm::radians(0.1f), glm::vec3(0.0f, 0.0f, 1.0f));
        }
        if (window.is_key_down(mve::InputKey::right)) {
            model = glm::rotate(model, glm::radians(-0.1f), glm::vec3(0.0f, 0.0f, 1.0f));
        }

        renderer.update_uniform(uniform_handle, model_location, model, false);

        renderer.begin(window);

        renderer.bind_graphics_pipeline(graphics_pipeline);

        renderer.bind_descriptor_set(descriptor_set_handle);

        renderer.bind_vertex_buffer(model_vertex_buffer);

        renderer.draw_index_buffer(model_index_buffer);

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
