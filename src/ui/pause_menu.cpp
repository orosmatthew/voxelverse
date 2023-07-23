#include "pause_menu.hpp"
#include "../logger.hpp"
#include "../common.hpp"

PauseMenu::PauseMenu(UIPipeline& ui_pipeline, TextPipeline& text_pipeline)
    : m_button_texture(std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray.png")))
    , m_button_texture_hover(std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray_hover.png")))
    , m_button_texture_pressed(
          std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray_pressed.png")))
    , m_back_button(ui_pipeline, text_pipeline, m_button_texture, "Back to game", { 100, 15 }, 5.0f)
    , m_fullscreen_button(ui_pipeline, text_pipeline, m_button_texture, "Toggle Fullscreen", { 100, 15 }, 5.0f)
    , m_exit_button(ui_pipeline, text_pipeline, m_button_texture, "Exit", { 100, 15 }, 5.0f)
{
    MVE_VAL_ASSERT(&ui_pipeline.renderer() == &text_pipeline.renderer(), "[PauseMenu] Renderers are not the same")

    m_exit_button.set_hover_texture(m_button_texture_hover);
    m_exit_button.set_pressed_texture(m_button_texture_pressed);
    m_back_button.set_hover_texture(m_button_texture_hover);
    m_back_button.set_pressed_texture(m_button_texture_pressed);
    m_fullscreen_button.set_hover_texture(m_button_texture_hover);
    m_fullscreen_button.set_pressed_texture(m_button_texture_pressed);

    resize(ui_pipeline.renderer().extent());
}
void PauseMenu::draw() const
{
    m_back_button.draw();
    m_fullscreen_button.draw();
    m_exit_button.draw();
}
void PauseMenu::resize(mve::Vector2i extent)
{
    m_back_button.set_position(
        (mve::Vector2(extent / 2.0f) - m_exit_button.size() / 2.0f) + mve::Vector2(0.0f, -100.0f));
    m_fullscreen_button.set_position(
        (mve::Vector2(extent / 2.0f) - m_exit_button.size() / 2.0f) + mve::Vector2(0.0f, 0.0f));
    m_exit_button.set_position(
        (mve::Vector2(extent / 2.0f) - m_exit_button.size() / 2.0f) + mve::Vector2(0.0f, 100.0f));
}
void PauseMenu::update(const mve::Window& window)
{
    m_back_button.update(window);
    m_fullscreen_button.update(window);
    m_exit_button.update(window);
}
