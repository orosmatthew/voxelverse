#pragma once

#include <filesystem>

#include "mve/renderer.hpp"
#include "ui_pipeline.hpp"

struct NinePatchMargins {
    int top;
    int bottom;
    int left;
    int right;
};

class NinePatch {
public:
    NinePatch(
        mve::Renderer& renderer,
        UIPipeline& ui_pipeline,
        const std::filesystem::path& img_path,
        NinePatchMargins margins);

    void draw(mve::Renderer& renderer, const UIPipeline& ui_pipeline);

private:
    NinePatchMargins m_margins;
    UIUniformData m_uniform_data;
    mve::VertexBuffer m_vertex_buffer;
    mve::IndexBuffer m_index_buffer;
    mve::Texture m_texture;
};