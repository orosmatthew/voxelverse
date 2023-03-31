#pragma once

#include "../mve/renderer.hpp"

#include "../ui_pipeline.hpp"

class Crosshair {
public:
    explicit Crosshair(std::shared_ptr<UIPipeline> pipeline);

    void draw() const;

private:
    std::weak_ptr<UIPipeline> m_pipeline;
    UIUniformData m_uniform_data;
    mve::Texture m_texture;
    mve::VertexBuffer m_vertex_buffer;
    mve::IndexBuffer m_index_buffer;
};