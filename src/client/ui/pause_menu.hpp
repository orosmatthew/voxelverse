#pragma once

#include <mve/window.hpp>

#include "button.hpp"
#include "options_menu.hpp"

class PauseMenu {
public:
    PauseMenu(UIPipeline& ui_pipeline, TextPipeline& text_pipeline);

    void draw() const;

    void resize(nnm::Vector2i extent);

    void update(mve::Window& window, mve::Renderer& renderer);

    [[nodiscard]] bool exit_pressed() const
    {
        return m_exit_button.is_pressed();
    }

    [[nodiscard]] bool should_close() const
    {
        return m_should_close;
    }

private:
    enum class State { pause, options };

    std::shared_ptr<mve::Texture> m_button_texture;
    std::shared_ptr<mve::Texture> m_button_texture_hover;
    std::shared_ptr<mve::Texture> m_button_texture_pressed;
    nnm::Vector2i m_extent;
    Button m_back_button;
    Button m_options_button;
    Button m_exit_button;
    OptionsMenu m_options_menu;
    State m_state;
    bool m_should_close;
};