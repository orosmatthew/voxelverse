#pragma once

#include "button.hpp"

class PauseMenu {
public:
    PauseMenu(UIPipeline& ui_pipeline, TextPipeline& text_pipeline);

    void draw() const;

    void resize(mve::Vector2i extent);

    void update(const mve::Window& window);

    [[nodiscard]] bool exit_pressed() const
    {
        return m_exit_button.is_pressed();
    }

    [[nodiscard]] bool back_pressed() const
    {
        return m_back_button.is_pressed();
    }

    [[nodiscard]] bool fullscreen_toggled() const
    {
        return m_fullscreen_button.is_pressed();
    }

private:
    std::shared_ptr<mve::Texture> m_button_texture;
    std::shared_ptr<mve::Texture> m_button_texture_hover;
    std::shared_ptr<mve::Texture> m_button_texture_pressed;
    Button m_back_button;
    Button m_fullscreen_button;
    Button m_exit_button;
};