#include "app.hpp"

#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtx/hash.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "logger.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "uniform_struct_layout.hpp"
#include "window.hpp"

namespace app {

std::pair<mve::VertexData, std::vector<uint32_t>> load_model(const std::filesystem::path& path)
{

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning;
    std::string error;

    std::string path_str = path.string();
    bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, path_str.c_str());
    if (!result) {
        throw std::runtime_error("[TinyObj] " + warning + " " + error);
    }

    mve::VertexLayout vertex_layout {};
    vertex_layout.push_back(mve::VertexAttributeType::vec3); // 3D position
    vertex_layout.push_back(mve::VertexAttributeType::vec3); // Color
    vertex_layout.push_back(mve::VertexAttributeType::vec2); // Texture coord

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texture_coord;

        bool operator==(const Vertex& other) const
        {
            return position == other.position && color == other.color && texture_coord == other.texture_coord;
        }
    };

    std::vector<Vertex> vertices {};
    std::vector<uint32_t> indices {};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex {};
            vertex.position = { attrib.vertices[3 * index.vertex_index + 0],
                                attrib.vertices[3 * index.vertex_index + 1],
                                attrib.vertices[3 * index.vertex_index + 2] };
            vertex.color = { 1.0f, 1.0f, 1.0f };
            vertex.texture_coord = { attrib.texcoords[2 * index.texcoord_index + 0],
                                     1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

            std::optional<size_t> duplicate_index;
            for (size_t i = 0; i < vertices.size(); i++) {
                if (vertex == vertices.at(i)) {
                    duplicate_index = i;
                    break;
                }
            }

            if (duplicate_index.has_value()) {
                indices.push_back(static_cast<uint32_t>(duplicate_index.value()));
            }
            else {
                indices.push_back(static_cast<uint32_t>(vertices.size()));
                vertices.push_back(vertex);
            }
        }
    }

    mve::VertexData data(vertex_layout);
    for (const Vertex& vertex : vertices) {
        data.push_back(vertex.position);
        data.push_back(vertex.color);
        data.push_back(vertex.texture_coord);
    }
    return { data, indices };
}

void run()
{
    LOG->debug("Creating window");

    mve::Window window("Mini Vulkan Engine", glm::ivec2(800, 600));

    window.set_min_size({ 800, 600 });

    mve::Renderer renderer(window, "Vulkan Testing", 0, 0, 1);

    mve::Shader vertex_shader("../res/bin/shader/simple.vert.spv", mve::ShaderType::vertex);
    mve::Shader fragment_shader("../res/bin/shader/simple.frag.spv", mve::ShaderType::fragment);

    mve::DescriptorSetLayoutHandle descriptor_set_layout_handle = renderer.create_descriptor_set_layout(
        { mve::DescriptorType::e_uniform_buffer, mve::DescriptorType::e_combined_image_sampler });

    mve::GraphicsPipelineLayoutHandle graphics_pipeline_layout
        = renderer.create_graphics_pipeline_layout({ descriptor_set_layout_handle });

    mve::VertexLayout vertex_layout {};
    vertex_layout.push_back(mve::VertexAttributeType::vec3); // 3D position
    vertex_layout.push_back(mve::VertexAttributeType::vec3); // Color
    vertex_layout.push_back(mve::VertexAttributeType::vec2); // Texture coord

    auto [model_vertices, model_indices] = load_model("../res/viking_room.obj");

    mve::VertexBufferHandle model_vertex_buffer = renderer.create_vertex_buffer(model_vertices);
    mve::IndexBufferHandle model_index_buffer = renderer.create_index_buffer(model_indices);

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

    mve::TextureHandle texture = renderer.create_texture("../res/viking_room.png", descriptor_set_handle, 1);

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

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        renderer.update_uniform(uniform_handle, model_location, model, false);

        renderer.begin(window);

        renderer.bind_graphics_pipeline(graphics_pipeline);

        renderer.bind_descriptor_set(descriptor_set_handle, graphics_pipeline_layout);

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
