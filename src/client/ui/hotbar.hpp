#pragma once

#include <mve/renderer.hpp>

#include "../ui_pipeline.hpp"

class Hotbar {
public:
    explicit Hotbar(UIPipeline& ui_pipeline);

    void resize(const nnm::Vector2i& extent);

    void update_hotbar_select(int pos);

    void draw() const;

    void set_item(int pos, uint8_t block_type);

    [[nodiscard]] std::optional<uint8_t> item_at(const int pos) const
    {
        if (m_items.at(pos).has_value()) {
            return m_items.at(pos)->type;
        }
        return {};
    }

    [[nodiscard]] int select_pos() const
    {
        return m_select_pos;
    }

private:
    struct Element {
        UIUniformData uniform_data;
        mve::VertexBuffer vertex_buffer;
        mve::IndexBuffer index_buffer;
    };

    struct Item {
        uint8_t type;
        Element element;
    };

    UIPipeline* m_ui_pipeline;

    mve::UniformLocation m_model_location;
    mve::ShaderDescriptorBinding m_texture_binding;

    mve::Texture m_hotbar_texture;
    mve::Texture m_select_texture;
    mve::Texture m_atlas_texture;

    Element m_hotbar;
    Element m_select;
    std::unordered_map<int, std::optional<Item>> m_items;

    nnm::Vector2i m_renderer_extent;
    int m_select_pos;

    [[nodiscard]] nnm::Vector3f scale() const;
    [[nodiscard]] nnm::Vector3f translation() const;
    static std::pair<mve::VertexData, std::vector<uint32_t>> create_item_mesh(uint8_t block_type);
};