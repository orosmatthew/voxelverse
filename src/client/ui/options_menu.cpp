#include "options_menu.hpp"

#include <mve/window.hpp>

#include "../common.hpp"
#include "../options.hpp"
#include "../ui_pipeline.hpp"

OptionsMenu::OptionsMenu(UIPipeline& ui_pipeline, TextPipeline& text_pipeline)
    : m_button_texture(std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray.png")))
    , m_button_texture_hover(std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray_hover.png")))
    , m_button_texture_pressed(
          std::make_shared<mve::Texture>(ui_pipeline.renderer(), res_path("button_gray_pressed.png")))
    , m_fullscreen_button(ui_pipeline, text_pipeline, m_button_texture, "Fullscreen: ", { 100, 15 }, 5.0f)
    , m_back_button(ui_pipeline, text_pipeline, m_button_texture, "Back", { 100, 15 }, 5.0f)
    , m_aa_button(ui_pipeline, text_pipeline, m_button_texture, "AA", { 100, 15 }, 5.0f)
    , m_should_close(false)
{
    m_fullscreen_button.set_hover_texture(m_button_texture_hover);
    m_fullscreen_button.set_pressed_texture(m_button_texture_pressed);
    m_back_button.set_hover_texture(m_button_texture_hover);
    m_back_button.set_pressed_texture(m_button_texture_pressed);
    m_aa_button.set_hover_texture(m_button_texture_hover);
    m_aa_button.set_pressed_texture(m_button_texture_pressed);
    resize(ui_pipeline.renderer().extent());
}

void OptionsMenu::draw() const
{
    m_fullscreen_button.draw();
    m_aa_button.draw();
    m_back_button.draw();
}

void OptionsMenu::resize(const mve::Vector2i extent)
{
    m_fullscreen_button.set_position(
        mve::Vector2f(extent / 2.0f) - m_fullscreen_button.size() / 2.0f + mve::Vector2(0.0f, -100.0f));
    m_aa_button.set_position(
        mve::Vector2f(extent / 2.0f) - m_fullscreen_button.size() / 2.0f + mve::Vector2(0.0f, 0.0f));
    m_back_button.set_position(
        mve::Vector2f(extent / 2.0f) - m_fullscreen_button.size() / 2.0f + mve::Vector2(0.0f, 100.0f));
}

void OptionsMenu::update(mve::Window& window, mve::Renderer& renderer)
{
    m_fullscreen_button.set_text(window.is_fullscreen() ? "Fullscreen: On" : "Fullscreen: Off");
    m_fullscreen_button.update(window);
    if (m_fullscreen_button.is_pressed()) {
        window.is_fullscreen() ? window.windowed() : window.fullscreen(true);
    }
    m_aa_button.update(window);
    if (m_aa_button.is_pressed()) {
        int new_msaa_i = static_cast<int>(renderer.current_msaa_samples()) + 1;
        if (new_msaa_i > static_cast<int>(renderer.max_msaa_samples())) {
            new_msaa_i = 0;
        }
        renderer.set_msaa_samples(window, static_cast<mve::Msaa>(new_msaa_i));
    }
    switch (renderer.current_msaa_samples()) {
    case mve::Msaa::samples_1:
        m_aa_button.set_text("MSAA: Off");
        break;
    case mve::Msaa::samples_2:
        m_aa_button.set_text("MSAA: 2x");
        break;
    case mve::Msaa::samples_4:
        m_aa_button.set_text("MSAA: 4x");
        break;
    case mve::Msaa::samples_8:
        m_aa_button.set_text("MSAA: 8x");
        break;
    case mve::Msaa::samples_16:
        m_aa_button.set_text("MSAA: 16x");
        break;
    case mve::Msaa::samples_32:
        m_aa_button.set_text("MSAA: 32x");
        break;
    case mve::Msaa::samples_64:
        m_aa_button.set_text("MSAA: 64x");
        break;
    }
    m_back_button.update(window);
    m_should_close = m_back_button.is_pressed() || window.is_key_pressed(mve::Key::escape);
    if (m_should_close) {
        const Options options { .fullscreen = window.is_fullscreen(), .msaa = renderer.current_msaa_samples() };
        set_options(options);
    }
}