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

static const mve::VertexLayout standard_vertex_layout = {
    mve::VertexAttributeType::vec3, // Position
    mve::VertexAttributeType::vec3, // Color
    mve::VertexAttributeType::vec2 // UV
};

struct MeshData {
    std::vector<mve::Vector3> vertices;
    std::vector<mve::Vector2> uvs;
    std::vector<uint32_t> indices;
};

MeshData create_quad_mesh(
    mve::Vector3 pos_top_left,
    mve::Vector3 pos_top_right,
    mve::Vector3 pos_bottom_right,
    mve::Vector3 pos_bottom_left,
    mve::Vector2 uv_top_left,
    mve::Vector2 uv_top_right,
    mve::Vector2 uv_bottom_right,
    mve::Vector2 uv_bottom_left)
{
    MeshData mesh_data;
    mesh_data.vertices.push_back(pos_top_left);
    mesh_data.uvs.push_back(uv_top_left);

    mesh_data.vertices.push_back(pos_top_right);
    mesh_data.uvs.push_back(uv_top_right);

    mesh_data.vertices.push_back(pos_bottom_right);
    mesh_data.uvs.push_back(uv_bottom_right);

    mesh_data.vertices.push_back(pos_bottom_left);
    mesh_data.uvs.push_back(uv_bottom_left);

    mesh_data.indices = { 0, 2, 3, 0, 1, 2 };

    return mesh_data;
}

void push_data(MeshData& data, const MeshData& face)
{
    uint32_t indices_offset = data.vertices.size();
    for (int i = 0; i < face.vertices.size(); i++) {
        data.vertices.push_back(face.vertices[i]);
        data.uvs.push_back(face.uvs[i]);
    }

    for (int i = 0; i < face.indices.size(); i++) {
        data.indices.push_back(face.indices[i] + indices_offset);
    }
}

MeshData create_cube_mesh()
{
    MeshData data;

    std::array<mve::Vector3, 4> quad_verts { mve::Vector3(-0.5f, 0.5f, 0.5f),
                                             mve::Vector3(0.5f, 0.5f, 0.5f),
                                             mve::Vector3(0.5f, 0.5f, -0.5f),
                                             mve::Vector3(-0.5f, 0.5f, -0.5f) };

    MeshData front_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0, 0 }, { 0.5f, 0 }, { 0.5f, 0.5f }, { 0, 0.5f });
    push_data(data, front_face_data);

    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(0, 0, 1), mve::pi / 2.0f);
    }

    MeshData right_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0, 0 }, { 0.5f, 0 }, { 0.5f, 0.5f }, { 0, 0.5f });
    push_data(data, right_face_data);

    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(0, 0, 1), mve::pi / 2.0f);
    }

    MeshData back_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0, 0 }, { 0.5f, 0 }, { 0.5f, 0.5f }, { 0, 0.5f });
    push_data(data, back_face_data);

    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(0, 0, 1), mve::pi / 2.0f);
    }

    MeshData left_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0, 0 }, { 0.5f, 0 }, { 0.5f, 0.5f }, { 0, 0.5f });
    push_data(data, left_face_data);

    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(0, 0, 1), mve::pi / 2.0f);
    }
    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(1, 0, 0), mve::pi / 2.0f);
    }

    MeshData top_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0.5f, 0 }, { 1, 0 }, { 1, 0.5f }, { 0.5f, 0.5f });
    push_data(data, top_face_data);

    for (int i = 0; i < 4; i++) {
        quad_verts[i] = quad_verts[i].rotate(mve::Vector3(1, 0, 0), mve::pi);
    }

    MeshData bottom_face_data = create_quad_mesh(
        quad_verts[0], quad_verts[1], quad_verts[2], quad_verts[3], { 0, 0.5f }, { 0.5f, 0.5f }, { 0.5f, 1 }, { 0, 1 });
    push_data(data, bottom_face_data);

    return data;
}

struct RenderObject {
    mve::DescriptorSet descriptor_set;
    mve::UniformBuffer ubo;
    mve::VertexBuffer vertex_buffer;
    mve::IndexBuffer index_buffer;
    mve::Texture texture;
    mve::UniformLocation model_location;
};

void run()
{
    LOG->debug("Creating window");

    mve::Window window("Mini Vulkan Engine", mve::Vector2i(800, 600));

    window.set_min_size({ 800, 600 });

    window.disable_cursor();

    mve::Renderer renderer(window, "Vulkan Testing", 0, 0, 1);

    std::vector<RenderObject> render_objects;

    mve::Shader vertex_shader("../res/bin/shader/simple.vert.spv", mve::ShaderType::vertex);
    mve::Shader fragment_shader("../res/bin/shader/simple.frag.spv", mve::ShaderType::fragment);

    mve::ModelData model_data = mve::load_model("../res/viking_room.obj");

    mve::GraphicsPipeline graphics_pipeline
        = renderer.create_graphics_pipeline(vertex_shader, fragment_shader, model_data.vertex_data.layout());

    RenderObject viking_scene {
        .descriptor_set = graphics_pipeline.create_descriptor_set(vertex_shader.descriptor_set(1)),
        .ubo = renderer.create_uniform_buffer(vertex_shader.descriptor_set(1).binding(0)),
        .vertex_buffer = renderer.create_vertex_buffer(model_data.vertex_data),
        .index_buffer = renderer.create_index_buffer(model_data.indices),
        .texture = renderer.create_texture("../res/viking_room.png"),
        .model_location = vertex_shader.descriptor_set(1).binding(0).member("model").location()
    };
    viking_scene.descriptor_set.write_binding(vertex_shader.descriptor_set(1).binding(0), viking_scene.ubo);
    viking_scene.descriptor_set.write_binding(fragment_shader.descriptor_set(1).binding(1), viking_scene.texture);
    viking_scene.ubo.update(viking_scene.model_location, mve::Matrix4::identity());

    //    render_objects.push_back(std::move(viking_scene));

    MeshData quad_mesh = create_cube_mesh();
    //= create_quad_mesh({ -1, 0, 1 }, { 1, 0, 1 }, { -1, 0, -1 }, { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 });

    mve::VertexData quad_vertex_data(standard_vertex_layout);
    for (int i = 0; i < quad_mesh.vertices.size(); i++) {
        quad_vertex_data.push_back(quad_mesh.vertices.at(i));
        quad_vertex_data.push_back({ 1, 1, 1 });
        quad_vertex_data.push_back(quad_mesh.uvs.at(i));
    }

    RenderObject grass_scene {
        .descriptor_set = graphics_pipeline.create_descriptor_set(vertex_shader.descriptor_set(1)),
        .ubo = renderer.create_uniform_buffer(vertex_shader.descriptor_set(1).binding(0)),
        .vertex_buffer = renderer.create_vertex_buffer(quad_vertex_data),
        .index_buffer = renderer.create_index_buffer(quad_mesh.indices),
        .texture = renderer.create_texture("../res/atlas.png"),
        .model_location = vertex_shader.descriptor_set(1).binding(0).member("model").location()
    };
    grass_scene.descriptor_set.write_binding(vertex_shader.descriptor_set(1).binding(0), grass_scene.ubo);
    grass_scene.descriptor_set.write_binding(fragment_shader.descriptor_set(1).binding(1), grass_scene.texture);
    grass_scene.ubo.update(grass_scene.model_location, mve::Matrix4::identity());

    render_objects.push_back(std::move(grass_scene));

    mve::UniformBuffer global_ubo = renderer.create_uniform_buffer(vertex_shader.descriptor_set(0).binding(0));

    mve::DescriptorSet global_descriptor_set
        = renderer.create_descriptor_set(graphics_pipeline, vertex_shader.descriptor_set(0));

    global_descriptor_set.write_binding(vertex_shader.descriptor_set(0).binding(0), global_ubo);

    std::chrono::high_resolution_clock::time_point begin_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;

    mve::UniformLocation view_location = vertex_shader.descriptor_set(0).binding(0).member("view").location();
    mve::UniformLocation proj_location = vertex_shader.descriptor_set(0).binding(0).member("proj").location();

    auto resize_func = [&](mve::Vector2i new_size) {
        renderer.resize(window);

        mve::Matrix4 my_proj = mve::perspective(
            mve::radians(90.0f), (float)renderer.extent().x / (float)renderer.extent().y, 0.01f, 1000.0f);
        global_ubo.update(proj_location, my_proj);
    };

    window.set_resize_callback(resize_func);

    std::invoke(resize_func, window.size());

    mve::Matrix4 model = mve::Matrix4().rotate(mve::Vector3(0.0f, 0.0f, 1.0f), mve::radians(90.0f));
    mve::Matrix4 prev_model = model;

    const float camera_acceleration = 0.01f;
    const float camera_speed = 0.1f;
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

    global_ubo.update(view_location, view);

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
                model = model.rotate(mve::Vector3(0.0f, 0.0f, 1.0f), mve::radians(1.0f));
            }
            if (window.is_key_down(mve::Key::right)) {
                model = model.rotate(mve::Vector3(0.0f, 0.0f, 1.0f), mve::radians(-1.0f));
            }
        });

        render_objects[0].ubo.update(
            render_objects[0].model_location, prev_model.interpolate(model, fixed_loop.blend()));

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

        global_ubo.update(view_location, view);
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

        renderer.begin(window);

        renderer.bind_graphics_pipeline(graphics_pipeline);

        for (const RenderObject& object : render_objects) {
            renderer.bind_descriptor_sets({ global_descriptor_set, object.descriptor_set });
            renderer.bind_vertex_buffer(object.vertex_buffer);
            renderer.draw_index_buffer(object.index_buffer);
        }

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
