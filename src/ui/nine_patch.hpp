#pragma once

#include <filesystem>

#include "../mve/renderer.hpp"
#include "../ui_pipeline.hpp"

struct NinePatchMargins {
    int top;
    int bottom;
    int left;
    int right;
};

class NinePatch {
public:
    NinePatch(
        UIPipeline& ui_pipeline,
        const std::filesystem::path& img_path,
        NinePatchMargins margins,
        mve::Vector2i size,
        float scale = 1.0f);

    void draw() const;

    void set_position(const mve::Vector2& pos);

    void set_scale(float scale);

    [[nodiscard]] inline mve::Vector2 position() const
    {
        return m_position;
    }

    [[nodiscard]] inline float scale() const
    {
        return m_scale;
    }

    [[nodiscard]] inline mve::Vector2i size() const
    {
        return m_size;
    }

private:
    UIPipeline* m_pipeline;
    UIUniformData m_uniform_data;
    mve::VertexBuffer m_vertex_buffer;
    mve::IndexBuffer m_index_buffer;
    mve::Texture m_texture;
    mve::Vector2 m_position;
    float m_scale;
    mve::Vector2i m_size;
};