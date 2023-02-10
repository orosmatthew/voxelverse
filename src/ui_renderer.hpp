#pragma once

#include "renderer.hpp"

class UIRenderer {
public:
    UIRenderer(mve::Renderer& renderer);

    void resize();

    void update_framebuffer_texture(const mve::Texture& texture, mve::Vector2i size);

    void draw();

private:
    const mve::VertexLayout c_vertex_layout = {
        mve::VertexAttributeType::vec3, // Position
        mve::VertexAttributeType::vec3, // Color
        mve::VertexAttributeType::vec2 // UV
    };

    struct Cross {
        mve::VertexBuffer vertex_buffer;
        mve::IndexBuffer index_buffer;
        mve::Texture texture;
        mve::DescriptorSet descriptor_set;
        mve::UniformBuffer uniform_buffer;
        mve::UniformLocation model_location;
    };

    struct World {
        mve::VertexBuffer vertex_buffer;
        mve::IndexBuffer index_buffer;
        mve::DescriptorSet descriptor_set;
        mve::UniformBuffer uniform_buffer;
        mve::UniformLocation model_location;
    };

    mve::Renderer* m_renderer;
    mve::Shader m_vertex_shader;
    mve::Shader m_fragment_shader;
    mve::GraphicsPipeline m_graphics_pipeline;
    mve::UniformBuffer m_global_ubo;
    mve::DescriptorSet m_global_descriptor_set;
    mve::UniformLocation m_view_location;
    mve::UniformLocation m_proj_location;

    std::optional<Cross> m_cross;
    std::optional<World> m_world;
};