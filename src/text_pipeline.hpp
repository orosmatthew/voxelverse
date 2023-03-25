#pragma once

#include "mve/renderer.hpp"

class TextBuffer;

class TextPipeline {
public:
    explicit TextPipeline(mve::Renderer& renderer);

    void resize();

    TextBuffer create_text_buffer();

    void update_text_buffer(const TextBuffer& buffer, std::string_view text, mve::Vector2 pos, float scale);

    void set_text_buffer_translation(const TextBuffer& buffer, mve::Vector2 pos);

    void draw(const TextBuffer& buffer) const;

    void destroy(TextBuffer& buffer);

private:
    struct FontChar {
        mve::Texture texture;
        mve::Vector2i size;
        mve::Vector2i bearing;
        uint32_t advance;
    };

    struct RenderGlyph {
        bool is_valid;
        mve::UniformBuffer ubo;
        mve::DescriptorSet descriptor_set;
        char character;
        mve::Vector2 translation;
        float scale;
    };

    using TextBufferImpl = std::vector<RenderGlyph>;

    const mve::VertexLayout c_vertex_layout = {
        mve::VertexAttributeType::vec3, // Position
        mve::VertexAttributeType::vec2 // UV
    };

    mve::Renderer* m_renderer;
    mve::Shader m_vert_shader;
    mve::Shader m_frag_shader;
    mve::GraphicsPipeline m_pipeline;
    mve::UniformBuffer m_global_ubo;
    mve::DescriptorSet m_global_descriptor_set;
    mve::VertexBuffer m_vertex_buffer;
    mve::IndexBuffer m_index_buffer;

    mve::UniformLocation m_model_location;
    mve::UniformLocation m_text_color_location;
    mve::ShaderDescriptorBinding m_texture_binding;
    mve::ShaderDescriptorBinding m_glyph_ubo_binding;

    std::unordered_map<char, FontChar> m_font_chars {};
    std::vector<std::optional<TextBufferImpl>> m_text_buffers {};
};
