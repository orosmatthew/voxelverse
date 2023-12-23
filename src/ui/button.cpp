#include "button.hpp"

#include "../logger.hpp"

#include <mve/window.hpp>

Button::Button(
    UIPipeline& ui_pipeline,
    TextPipeline& text_pipeline,
    const std::shared_ptr<mve::Texture>& texture,
    const std::string& text,
    const mve::Vector2i& size,
    const float scale)
    : m_text_pipeline(&text_pipeline)
    , m_patch(ui_pipeline, texture, { 2, 2, 2, 2 }, size, scale)
    , m_text(text_pipeline, text, { 0, 0 }, 1.0f, { 0.0f, 0.0f, 0.0f })
    , m_position(mve::Vector2(0.0f, 0.0f))
    , m_texture(texture)
    , m_texture_pressed(texture)
    , m_texture_hover(texture)
    , m_prev_state(State::none)
    , m_state(State::none)
    , m_scale(scale)
{
    set_position({ 0, 0 });
}

void Button::draw() const
{
    m_patch.draw();
    m_text.draw();
}
void Button::set_position(const mve::Vector2& pos)
{
    m_patch.set_position(pos);
    m_text.set_translation(
        { pos.x + static_cast<float>(m_patch.size().x) * m_patch.scale() / 2.0f - m_text.width() / 2.0f,
          pos.y + static_cast<float>(m_patch.size().y) * m_patch.scale() / 2.0f
              - static_cast<float>(m_text_pipeline->point_size()) / 2.0f });
}
void Button::set_scale(const float scale)
{
    m_patch.set_scale(scale);
    m_text.set_scale(scale);
}

void Button::set_text(const std::string& text)
{
    m_text.update(text);
}

void Button::set_hover_texture(std::shared_ptr<mve::Texture> texture)
{
    m_texture_hover = std::move(texture);
}
void Button::update(const mve::Window& window)
{
    const mve::Vector2 mouse_pos = window.mouse_pos();
    const bool hover = is_pos_in_button(mouse_pos);
    const bool pressed = window.is_mouse_button_pressed(mve::MouseButton::left);

    m_prev_state = m_state;
    switch (m_state) {
    case State::none:
        if (hover && pressed) {
            m_state = State::down;
            m_patch.update_texture(*m_texture_pressed);
        }
        else if (hover) {
            m_state = State::hover;
            m_patch.update_texture(*m_texture_hover);
        }
        break;
    case State::hover:
        if (!hover) {
            m_state = State::none;
            m_patch.update_texture(*m_texture);
        }
        else if (pressed) {
            m_state = State::down;
            m_patch.update_texture(*m_texture_pressed);
        }
        break;
    case State::down:
        if (!hover) {
            m_state = State::none;
            m_patch.update_texture(*m_texture);
        }
        else if (!pressed) {
            m_state = State::hover;
            m_patch.update_texture(*m_texture_hover);
        }
        break;
    }
}
void Button::set_pressed_texture(std::shared_ptr<mve::Texture> texture)
{
    m_texture_pressed = std::move(texture);
}
bool Button::is_pos_in_button(const mve::Vector2& pos) const
{
    return pos.x >= m_patch.position().x && pos.y >= m_patch.position().y
        && pos.x <= m_patch.position().x + static_cast<float>(m_patch.size().x) * m_patch.scale()
        && pos.y <= m_patch.position().y + static_cast<float>(m_patch.size().y) * m_patch.scale();
}
