#include "pause_menu.hpp"

#include "../../common/assert.hpp"
#include "../../common/logger.hpp"
#include "../common.hpp"

PauseMenu::PauseMenu(UIPipeline& ui_pipeline, TextPipeline& text_pipeline)
    : m_button_texture(std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray.png")))
    , m_button_texture_hover(std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray_hover.png")))
    , m_button_texture_pressed(
          std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray_pressed.png")))
    , m_extent(ui_pipeline.renderer().extent())
    , m_back_button(ui_pipeline, text_pipeline, m_button_texture, "Back to game", { 100, 15 }, 5.0f)
    , m_options_button(ui_pipeline, text_pipeline, m_button_texture, "Options", { 100, 15 }, 5.0f)
    , m_exit_button(ui_pipeline, text_pipeline, m_button_texture, "Save & Exit", { 100, 15 }, 5.0f)
    , m_options_menu(ui_pipeline, text_pipeline)
    , m_state(State::pause)
    , m_should_close(false)
{
    VV_DEB_ASSERT(&ui_pipeline.renderer() == &text_pipeline.renderer(), "[PauseMenu] Renderers are not the same")

    m_exit_button.set_hover_texture(m_button_texture_hover);
    m_exit_button.set_pressed_texture(m_button_texture_pressed);
    m_back_button.set_hover_texture(m_button_texture_hover);
    m_back_button.set_pressed_texture(m_button_texture_pressed);
    m_options_button.set_hover_texture(m_button_texture_hover);
    m_options_button.set_pressed_texture(m_button_texture_pressed);

    resize(ui_pipeline.renderer().extent());
}

void PauseMenu::draw() const
{
    if (m_state == State::pause) {
        m_back_button.draw();
        m_options_button.draw();
        m_exit_button.draw();
    }
    else if (m_state == State::options) {
        m_options_menu.draw();
    }
}

void PauseMenu::resize(const nnm::Vector2i extent)
{
    m_extent = extent;
    if (m_state == State::pause) {
        m_back_button.set_position(
            nnm::Vector2f(nnm::Vector2f(extent) / 2.0f) - m_exit_button.size() / 2.0f + nnm::Vector2(0.0f, -100.0f));
        m_options_button.set_position(
            nnm::Vector2f(nnm::Vector2f(extent) / 2.0f) - m_exit_button.size() / 2.0f + nnm::Vector2(0.0f, 0.0f));
        m_exit_button.set_position(
            nnm::Vector2f(nnm::Vector2f(extent) / 2.0f) - m_exit_button.size() / 2.0f + nnm::Vector2(0.0f, 100.0f));
    }
    else if (m_state == State::options) {
        m_options_menu.resize(extent);
    }
}

void PauseMenu::update(mve::Window& window, mve::Renderer& renderer)
{
    if (m_state == State::pause) {
        m_back_button.update(window);
        m_options_button.update(window);
        m_exit_button.update(window);

        if (m_options_button.is_pressed()) {
            m_state = State::options;
            m_options_menu.resize(m_extent);
        }
        m_should_close = m_back_button.is_pressed() || window.is_key_pressed(mve::Key::escape);
    }
    else if (m_state == State::options) {
        m_options_menu.update(window, renderer);
        if (m_options_menu.should_close()) {
            m_state = State::pause;
            resize(m_extent);
        }
    }
}
