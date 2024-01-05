#pragma once

#include <mve/renderer.hpp>
#include <mve/window.hpp>

#include "../ui_pipeline.hpp"
#include "button.hpp"

class OptionsMenu {
public:
    explicit OptionsMenu(UIPipeline& ui_pipeline, TextPipeline& text_pipeline);

    void draw() const;

    void resize(mve::Vector2i extent);

    void update(mve::Window& window, mve::Renderer& renderer);

    [[nodiscard]] bool should_close() const
    {
        return m_should_close;
    }

private:
    std::shared_ptr<mve::Texture> m_button_texture;
    std::shared_ptr<mve::Texture> m_button_texture_hover;
    std::shared_ptr<mve::Texture> m_button_texture_pressed;
    Button m_fullscreen_button;
    Button m_back_button;
    Button m_aa_button;
    bool m_should_close;
};