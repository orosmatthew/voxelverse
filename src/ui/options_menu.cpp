#include "options_menu.hpp"

#include <mve/window.hpp>

#include "../common.hpp"
#include "../ui_pipeline.hpp"

OptionsMenu::OptionsMenu(UIPipeline& ui_pipeline, TextPipeline& text_pipeline)
    : m_button_texture(std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray.png")))
    , m_button_texture_hover(std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray_hover.png")))
    , m_button_texture_pressed(
          std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray_pressed.png")))
    , m_fullscreen_button(ui_pipeline, text_pipeline, m_button_texture, "Fullscreen: ", { 100, 15 }, 5.0f)
    , m_back_button(ui_pipeline, text_pipeline, m_button_texture, "Back", { 100, 15 }, 5.0f)
    , m_should_close(false)
{
    m_fullscreen_button.set_hover_texture(m_button_texture_hover);
    m_fullscreen_button.set_pressed_texture(m_button_texture_pressed);
    m_back_button.set_hover_texture(m_button_texture_hover);
    m_back_button.set_pressed_texture(m_button_texture_pressed);
    resize(ui_pipeline.renderer().extent());
}

void OptionsMenu::draw() const
{
    m_fullscreen_button.draw();
    m_back_button.draw();
}

void OptionsMenu::resize(const mve::Vector2i extent)
{
    m_fullscreen_button.set_position(
        mve::Vector2(extent / 2.0f) - m_fullscreen_button.size() / 2.0f + mve::Vector2(0.0f, 0.0f));
    m_back_button.set_position(
        mve::Vector2(extent / 2.0f) - m_fullscreen_button.size() / 2.0f + mve::Vector2(0.0f, 100.0f));
}

void OptionsMenu::update(mve::Window& window)
{
    m_fullscreen_button.set_text(window.is_fullscreen() ? "Fullscreen: On" : "Fullscreen: Off");
    m_fullscreen_button.update(window);
    if (m_fullscreen_button.is_pressed()) {
        window.is_fullscreen() ? window.windowed() : window.fullscreen(true);
    }
    m_back_button.update(window);
    m_should_close = m_back_button.is_pressed() || window.is_key_pressed(mve::Key::escape);
}