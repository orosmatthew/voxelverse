#include "app.hpp"

#include <chrono>

#include "logger.hpp"
#include "math/functions.hpp"
#include "math/matrix4.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "util.hpp"
#include "util/fixed_loop.hpp"
#include "window.hpp"

namespace app {

void run()
{
    LOG->debug("Creating window");

    mve::Window window("Mini Vulkan Engine", mve::Vector2i(800, 600));

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

    auto resize_func = [&](mve::Vector2i new_size) {
        renderer.resize(window);

        mve::Matrix4 my_proj = mve::perspective(
            mve::radians(90.0f), (float)renderer.extent().x / (float)renderer.extent().y, 0.01f, 10.0f);
        uniform_buffer.update(proj_location, my_proj);
    };

    window.set_resize_callback(resize_func);

    std::invoke(resize_func, window.size());

    mve::Texture texture = renderer.create_texture("../res/viking_room.png");

    descriptor_set.write_binding(fragment_shader.descriptor_set(0).binding(1), texture);

    mve::Matrix4 model = mve::Matrix4().rotate(mve::Vector3(0.0f, 0.0f, 1.0f), mve::radians(90.0f));
    mve::Matrix4 prev_model = model;

    const float camera_acceleration = 0.0045f;
    const float camera_speed = 0.05f;
    const float camera_friction = 0.1f;
    mve::Vector3 camera_pos(0.0f, 3.0f, 0.0f);
    mve::Vector3 camera_pos_prev = camera_pos;
    mve::Vector3 camera_front(0.0f, -1.0f, 0.0f);
    mve::Vector3 camera_up(0.0f, 0.0f, 1.0f);
    mve::Vector3 camera_velocity(0.0f);
    mve::Matrix4 view;
    float camera_yaw = 0.0f;
    float camera_pitch = 0.0f;
    view = mve::look_at(camera_pos, camera_pos + camera_front, camera_up);

    uniform_buffer.update(view_location, view);

    util::FixedLoop fixed_loop(60.0f);

    bool cursor_captured = true;

    while (!window.should_close()) {
        window.poll_events();

        fixed_loop.update(20, [&]() {
            mve::Vector3 dir(0.0f);
            if (window.is_key_down(mve::Key::w)) {
                dir.x += mve::cos(mve::radians(camera_yaw));
                dir.y += mve::sin(mve::radians(camera_yaw));
            }
            if (window.is_key_down(mve::Key::s)) {
                dir.x -= mve::cos(mve::radians(camera_yaw));
                dir.y -= mve::sin(mve::radians(camera_yaw));
            }
            if (window.is_key_down(mve::Key::a)) {
                dir.x += mve::cos(mve::radians(camera_yaw + 90.0f));
                dir.y += mve::sin(mve::radians(camera_yaw + 90.0f));
            }
            if (window.is_key_down(mve::Key::d)) {
                dir.x -= mve::cos(mve::radians(camera_yaw + 90.0f));
                dir.y -= mve::sin(mve::radians(camera_yaw + 90.0f));
            }
            if (window.is_key_down(mve::Key::space)) {
                dir.z += 1;
            }
            if (window.is_key_down(mve::Key::left_shift)) {
                dir.z -= 1;
            }
            camera_velocity -= (camera_velocity * camera_friction);
            camera_pos_prev = camera_pos;
            if (dir != mve::Vector3(0.0f)) {
                camera_velocity += dir.normalize() * camera_acceleration;
            }
            if (camera_velocity.length() > camera_speed) {
                camera_velocity = camera_velocity.normalize() * camera_speed;
            }
            camera_pos += camera_velocity;

            prev_model = model;
            if (window.is_key_down(mve::Key::left)) {
                model = model.translate(mve::Vector3(0.1f, 0.0f, 0.0f));
                model = model.rotate(mve::Vector3(0.0f, 0.0f, 1.0f), mve::radians(0.5f));
            }
            if (window.is_key_down(mve::Key::right)) {
                model = model.translate(mve::Vector3(-0.1f, 0.0f, 0.0f));
                model = model.rotate(mve::Vector3(0.0f, 0.0f, 1.0f), mve::radians(-0.5f));
            }

        });

        if (cursor_captured) {
            mve::Vector2 mouse_delta = window.mouse_delta();
            camera_yaw -= mouse_delta.x * 0.1f;
            camera_pitch -= mouse_delta.y * 0.1f;
        }

        camera_pitch = mve::clamp(camera_pitch, -89.0f, 89.0f);
        mve::Vector3 direction;
        direction.x = mve::cos(mve::radians(camera_yaw)) * mve::cos(mve::radians(camera_pitch));
        direction.y = mve::sin(mve::radians(camera_yaw)) * mve::cos(mve::radians(camera_pitch));
        direction.z = mve::sin(mve::radians(camera_pitch));
        camera_front = direction.normalize();

        mve::Vector3 pos = camera_pos_prev.linear_interpolate(camera_pos, fixed_loop.blend());
        view = mve::look_at(pos, pos + camera_front, camera_up);

        uniform_buffer.update(view_location, view);
        if (window.is_key_pressed(mve::Key::escape)) {
            break;
        }

        if (window.is_key_pressed(mve::Key::f)) {
            if (!window.is_fullscreen()) {
                window.fullscreen(true);
            }
            else {
                window.windowed();
            }
        }

        if (window.is_key_pressed(mve::Key::c)) {
            if (cursor_captured) {
                window.enable_cursor();
                cursor_captured = false;
            }
            else {
                window.disable_cursor();
                cursor_captured = true;
            }
        }

        uniform_buffer.update(model_location, prev_model.interpolate(model, fixed_loop.blend()), false);

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
