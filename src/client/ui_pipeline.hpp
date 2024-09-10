#pragma once

#include <array>

#include <mve/renderer.hpp>

struct UIUniformData {
    mve::DescriptorSet descriptor_set;
    mve::UniformBuffer buffer;
};

class UIPipeline {
public:
    explicit UIPipeline(mve::Renderer& renderer);

    void resize();

    void update_framebuffer_texture(const mve::Texture& texture, nnm::Vector2i size);

    void draw_world() const;

    void draw(
        const mve::DescriptorSet& descriptor_set,
        const mve::VertexBuffer& vertex_buffer,
        const mve::IndexBuffer& index_buffer) const;

    [[nodiscard]] mve::UniformLocation model_location() const
    {
        return m_vertex_shader.descriptor_set(1).binding(0).member("model").location();
    }

    [[nodiscard]] const mve::ShaderDescriptorBinding& texture_binding() const
    {
        return m_fragment_shader.descriptor_set(1).binding(1);
    }

    [[nodiscard]] const mve::DescriptorSet& global_descriptor_set() const
    {
        return m_global_descriptor_set;
    }

    [[nodiscard]] UIUniformData create_uniform_data() const;

    void bind() const;

    [[nodiscard]] mve::Renderer& renderer() const
    {
        return *m_renderer;
    }

    static std::array<mve::VertexAttributeType, 3> vertex_layout()
    {
        return {
            mve::VertexAttributeType::vector3, // Position
            mve::VertexAttributeType::vector3, // Color
            mve::VertexAttributeType::vector2 // UV
        };
    }

private:
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

    std::optional<World> m_world;
};