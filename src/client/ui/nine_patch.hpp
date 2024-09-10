#pragma once

#include <filesystem>

#include <mve/renderer.hpp>

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
        const std::shared_ptr<mve::Texture>& texture,
        NinePatchMargins margins,
        nnm::Vector2i size,
        float scale = 1.0f);

    void draw() const;

    void set_position(const nnm::Vector2f& pos);

    void set_scale(float scale);

    void update_texture(const mve::Texture& texture);

    [[nodiscard]] nnm::Vector2f position() const
    {
        return m_position;
    }

    [[nodiscard]] float scale() const
    {
        return m_scale;
    }

    [[nodiscard]] nnm::Vector2i size() const
    {
        return m_size;
    }

private:
    UIPipeline* m_pipeline;
    UIUniformData m_uniform_data;
    mve::VertexBuffer m_vertex_buffer;
    mve::IndexBuffer m_index_buffer;
    std::shared_ptr<mve::Texture> m_texture;
    mve::UniformLocation m_model_location;
    nnm::Vector2f m_position;
    float m_scale;
    nnm::Vector2i m_size;
};