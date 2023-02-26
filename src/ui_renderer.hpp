#pragma once

#include "mve/renderer.hpp"

class UIRenderer {
public:
    explicit UIRenderer(mve::Renderer& renderer);

    void resize();

    void update_framebuffer_texture(const mve::Texture& texture, mve::Vector2i size);

    void draw();

    void set_hotbar_select(int pos);

    void set_hotbar_block(int pos, uint8_t block_type);

private:
    const mve::VertexLayout c_vertex_layout = {
        mve::VertexAttributeType::vec3, // Position
        mve::VertexAttributeType::vec3, // Color
        mve::VertexAttributeType::vec2 // UV
    };

    struct UIMesh {
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

    struct MeshData {
        std::vector<mve::Vector3> vertices;
        std::vector<mve::Vector3> colors;
        std::vector<mve::Vector2> uvs;
        std::vector<uint32_t> indices;
    };

    mve::VertexData create_hotbar_vertex_data() const;

    mve::VertexData create_hotbar_select_vertex_data() const;

    MeshData create_block_face_data(uint8_t block_type) const;

    mve::Renderer* m_renderer;
    mve::Shader m_vertex_shader;
    mve::Shader m_fragment_shader;
    mve::GraphicsPipeline m_graphics_pipeline;
    mve::UniformBuffer m_global_ubo;
    mve::DescriptorSet m_global_descriptor_set;
    mve::UniformLocation m_view_location;
    mve::UniformLocation m_proj_location;

    std::optional<UIMesh> m_cross;
    std::optional<World> m_world;
    std::optional<UIMesh> m_hotbar;
    std::optional<UIMesh> m_hotbar_select;
    std::unordered_map<int, std::optional<UIMesh>> m_hotbar_blocks;
    int m_current_hotbar_select = 0;
};