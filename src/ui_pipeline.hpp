#pragma once

#include "mve/renderer.hpp"

struct UIUniformData {
    mve::DescriptorSet descriptor_set;
    mve::UniformBuffer buffer;
};

class UIPipeline {
public:
    explicit UIPipeline(mve::Renderer& renderer);

    void resize();

    void update_framebuffer_texture(const mve::Texture& texture, mve::Vector2i size);

    void draw();

    void draw_world() const;

    void draw(
        const mve::DescriptorSet& descriptor_set,
        const mve::VertexBuffer& vertex_buffer,
        const mve::IndexBuffer& index_buffer) const;

    inline mve::UniformLocation model_location() const
    {
        return m_vertex_shader.descriptor_set(1).binding(0).member("model").location();
    }

    inline const mve::ShaderDescriptorBinding& texture_binding() const
    {
        return m_fragment_shader.descriptor_set(1).binding(1);
    }

    inline void enable_debug()
    {
        m_show_debug = true;
        update_debug_glyphs();
    }

    inline void disable_debug()
    {
        m_show_debug = false;
    }

    [[nodiscard]] inline bool is_debug_enabled() const
    {
        return m_show_debug;
    }

    void update_fps(int value);

    inline void update_player_block_pos(mve::Vector3i block_pos)
    {
        m_player_block_pos = block_pos;
    }

    inline void update_player_chunk_pos(mve::Vector3i chunk_pos)
    {
        m_player_chunk_pos = chunk_pos;
    }

    inline void update_gpu_name(const std::string& name)
    {
        m_gpu_name = name;
    }

    inline const mve::DescriptorSet& global_descriptor_set() const
    {
        return m_global_descriptor_set;
    }

    UIUniformData create_uniform_data();

    void bind() const;

    inline mve::Renderer& renderer()
    {
        return *m_renderer;
    }

    static inline const mve::VertexLayout vertex_layout()
    {
        return {
            mve::VertexAttributeType::vec3, // Position
            mve::VertexAttributeType::vec3, // Color
            mve::VertexAttributeType::vec2 // UV
        };
    }

private:
    const mve::VertexLayout c_text_vertex_layout = {
        mve::VertexAttributeType::vec3, // Position
        mve::VertexAttributeType::vec2 // UV
    };

    struct World {
        mve::VertexBuffer vertex_buffer;
        mve::IndexBuffer index_buffer;
        mve::DescriptorSet descriptor_set;
        mve::UniformBuffer uniform_buffer;
        mve::UniformLocation model_location;
    };

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
    };

    void add_glyphs(std::vector<RenderGlyph>& glyphs, const std::string& text, mve::Vector2 pos, float scale);

    void update_debug_glyphs();

    mve::Renderer* m_renderer;
    mve::Shader m_vertex_shader;
    mve::Shader m_fragment_shader;
    mve::GraphicsPipeline m_graphics_pipeline;
    mve::UniformBuffer m_global_ubo;
    mve::DescriptorSet m_global_descriptor_set;
    mve::UniformLocation m_view_location;
    mve::UniformLocation m_proj_location;

    mve::Shader m_text_vert_shader;
    mve::Shader m_text_frag_shader;
    mve::GraphicsPipeline m_text_pipeline;
    mve::UniformBuffer m_text_ubo;
    mve::DescriptorSet m_text_descriptor_set;
    mve::VertexBuffer m_text_vertex_buffer;
    mve::IndexBuffer m_text_index_buffer;

    std::optional<World> m_world;

    std::unordered_map<char, FontChar> m_font_chars {};
    std::vector<RenderGlyph> m_debug_glyphs {};

    bool m_show_debug = false;
    int m_fps_value = 0;
    mve::Vector3i m_player_block_pos {};
    mve::Vector3i m_player_chunk_pos {};
    std::string m_gpu_name {};
};