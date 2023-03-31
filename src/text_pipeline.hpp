#pragma once

#include "mve/common.hpp"
#include "mve/renderer.hpp"

class TextBuffer;

class TextPipeline {
public:
    explicit TextPipeline(mve::Renderer& renderer, int point_size);

    void resize();

    TextBuffer create_text_buffer();

    TextBuffer create_text_buffer(std::string_view text, mve::Vector2 pos, float scale, mve::Vector3 color);

    void update_text_buffer(const TextBuffer& buffer, std::string_view text);

    void add_cursor(const TextBuffer& buffer, int pos);

    void set_cursor_pos(const TextBuffer& buffer, int pos);

    void cursor_right(const TextBuffer& buffer);

    void cursor_left(const TextBuffer& buffer);

    std::optional<int> cursor_pos(const TextBuffer& buffer);

    void remove_cursor(const TextBuffer& buffer);

    void set_text_buffer_translation(const TextBuffer& buffer, mve::Vector2 pos);

    void set_text_buffer_scale(const TextBuffer& buffer, float scale);

    void set_text_buffer_color(const TextBuffer& buffer, mve::Vector3 color);

    [[nodiscard]] float text_buffer_width(const TextBuffer& buffer) const;

    [[nodiscard]] inline float point_size() const
    {
        return c_point_size;
    }

    [[nodiscard]] inline mve::Renderer& renderer()
    {
        return *m_renderer;
    }

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

    struct Cursor {
        mve::UniformBuffer ubo;
        mve::DescriptorSet descriptor_set;
        mve::Vector2 translation;
        float scale;
    };

    struct TextBufferImpl {
        std::vector<RenderGlyph> render_glyphs;
        std::optional<Cursor> cursor;
        int cursor_pos;
        mve::Vector2 translation;
        float scale;
        int text_length;
        mve::Vector3 color;
    };

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
    mve::VertexBuffer m_cursor_vertex_buffer;
    mve::IndexBuffer m_cursor_index_buffer;
    mve::Texture m_cursor_texture;

    mve::UniformLocation m_model_location;
    mve::UniformLocation m_text_color_location;
    mve::ShaderDescriptorBinding m_texture_binding;
    mve::ShaderDescriptorBinding m_glyph_ubo_binding;

    std::unordered_map<char, FontChar> m_font_chars {};
    std::vector<std::optional<TextBufferImpl>> m_text_buffers {};
    const int c_point_size;
};
