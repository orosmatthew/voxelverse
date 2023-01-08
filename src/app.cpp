#include "app.hpp"

#include <chrono>

#include <glm/ext.hpp>

#include "logger.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "util.hpp"
#include "util/fixed_loop.hpp"
#include "window.hpp"

static glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t)
{
    return a * (1.0f - t) + b * t;
}

namespace app {

void run()
{
    LOG->debug("Creating window");

    mve::Window window("Mini Vulkan Engine", glm::ivec2(800, 600));

    window.set_min_size({ 800, 600 });

    window.disable_cursor();

    mve::Renderer renderer(window, "Vulkan Testing", 0, 0, 1);

    mve::Shader vertex_shader("../res/bin/shader/simple.vert.spv", mve::ShaderType::vertex);
    mve::Shader fragment_shader("../res/bin/shader/simple.frag.spv", mve::ShaderType::fragment);

    mve::ModelData model_data = mve::load_model("../res/viking_room.obj");

    mve::VertexBuffer model_vertex_buffer = renderer.create_vertex_buffer(model_data.vertex_data);
    mve::IndexBuffer model_index_buffer = renderer.create_index_buffer(model_data.indices);

    mve::GraphicsPipeline graphics_pipeline
        = renderer.create_graphics_pipeline(vertex_shader, fragment_shader, model_data.vertex_data.layout());

    mve::DescriptorSet descriptor_set = graphics_pipeline.create_descriptor_set(vertex_shader.descriptor_set(0));

    mve::UniformBuffer uniform_buffer = renderer.create_uniform_buffer(vertex_shader.descriptor_set(0).binding(0));

    descriptor_set.write_binding(vertex_shader.descriptor_set(0).binding(0), uniform_buffer);

    std::chrono::high_resolution_clock::time_point begin_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;

    mve::UniformLocation view_location = vertex_shader.descriptor_set(0).binding(0).member("view").location();
    mve::UniformLocation model_location = vertex_shader.descriptor_set(0).binding(0).member("model").location();
    mve::UniformLocation proj_location = vertex_shader.descriptor_set(0).binding(0).member("proj").location();

    auto resize_func = [&](glm::ivec2 new_size) {
        renderer.resize(window);
        glm::mat4 proj = glm::perspective(
            glm::radians(90.0f), (float)renderer.extent().x / (float)renderer.extent().y, 0.01f, 10.0f);
        proj[1][1] *= -1;
        uniform_buffer.update(proj_location, proj);
    };

    window.set_resize_callback(resize_func);

    std::invoke(resize_func, window.size());

    mve::Texture texture = renderer.create_texture("../res/viking_room.png");

    descriptor_set.write_binding(fragment_shader.descriptor_set(0).binding(1), texture);

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    const float camera_acceleration = 0.0045f;
    const float camera_speed = 0.05f;
    const float camera_friction = 0.1f;
    glm::vec3 camera_pos = glm::vec3(0.0f, 3.0f, 0.0f);
    glm::vec3 camera_pos_prev = camera_pos;
    glm::vec3 camera_front = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 camera_direction;
    glm::vec3 camera_up = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 camera_velocity = glm::vec3(0.0f);
    glm::mat4 view;
    float camera_yaw = 0.0f;
    float camera_pitch = 0.0f;
    view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);

    uniform_buffer.update(view_location, view);

    util::FixedLoop fixed_loop(60.0f);

    bool cursor_captured = true;

    while (!window.should_close()) {
        window.poll_events();

        fixed_loop.update(20, [&]() {
            glm::vec3 dir(0.0f);
            if (window.is_key_down(mve::InputKey::w)) {
                dir.x += glm::cos(glm::radians(camera_yaw));
                dir.y += glm::sin(glm::radians(camera_yaw));
            }
            if (window.is_key_down(mve::InputKey::s)) {
                dir.x -= glm::cos(glm::radians(camera_yaw));
                dir.y -= glm::sin(glm::radians(camera_yaw));
            }
            if (window.is_key_down(mve::InputKey::a)) {
                dir.x += glm::cos(glm::radians(camera_yaw + 90.0f));
                dir.y += glm::sin(glm::radians(camera_yaw + 90.0f));
            }
            if (window.is_key_down(mve::InputKey::d)) {
                dir.x -= glm::cos(glm::radians(camera_yaw + 90.0f));
                dir.y -= glm::sin(glm::radians(camera_yaw + 90.0f));
            }
            if (window.is_key_down(mve::InputKey::space)) {
                dir.z += 1;
            }
            if (window.is_key_down(mve::InputKey::left_shift)) {
                dir.z -= 1;
            }
            camera_velocity -= (camera_velocity * camera_friction);
            camera_pos_prev = camera_pos;
            if (dir != glm::vec3(0.0f)) {
                camera_velocity += glm::normalize(dir) * camera_acceleration;
            }
            if (glm::length(camera_velocity) > camera_speed) {
                camera_velocity = glm::normalize(camera_velocity) * camera_speed;
            }
            camera_pos += camera_velocity;

            if (window.is_key_down(mve::InputKey::left)) {
                model = glm::rotate(model, glm::radians(0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
            }
            if (window.is_key_down(mve::InputKey::right)) {
                model = glm::rotate(model, glm::radians(-0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
            }
        });

        if (cursor_captured) {
            glm::vec2 mouse_delta = window.mouse_delta();
            camera_yaw -= mouse_delta.x * 0.1f;
            camera_pitch -= mouse_delta.y * 0.1f;
        }

        camera_pitch = glm::clamp(camera_pitch, -89.0f, 89.0f);
        glm::vec3 direction;
        direction.x = glm::cos(glm::radians(camera_yaw)) * glm::cos(glm::radians(camera_pitch));
        direction.y = glm::sin(glm::radians(camera_yaw)) * glm::cos(glm::radians(camera_pitch));
        direction.z = glm::sin(glm::radians(camera_pitch));
        camera_front = glm::normalize(direction);

        glm::vec3 pos = lerp(camera_pos_prev, camera_pos, fixed_loop.blend());
        view = glm::lookAt(pos, pos + camera_front, camera_up);

        uniform_buffer.update(view_location, view);
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

        if (window.is_key_pressed(mve::InputKey::c)) {
            if (cursor_captured) {
                window.enable_cursor();
                cursor_captured = false;
            }
            else {
                window.disable_cursor();
                cursor_captured = true;
            }
        }

        uniform_buffer.update(model_location, model, false);

        renderer.begin(window);

        renderer.bind_graphics_pipeline(graphics_pipeline);

        renderer.bind_descriptor_set(descriptor_set);

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
