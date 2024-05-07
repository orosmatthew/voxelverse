#pragma once

#include "../text_buffer.hpp"
#include "nine_patch.hpp"

class Button {
public:
    explicit Button(
        UIPipeline& ui_pipeline,
        TextPipeline& text_pipeline,
        const std::shared_ptr<mve::Texture>& texture,
        const std::string& text,
        const mve::Vector2i& size,
        float scale = 1.0f);

    void draw() const;

    void set_position(const mve::Vector2f& pos);

    void set_scale(float scale);

    void set_text(const std::string& text);

    void update(const mve::Window& window);

    void set_hover_texture(std::shared_ptr<mve::Texture> texture);

    void set_pressed_texture(std::shared_ptr<mve::Texture> texture);

    [[nodiscard]] mve::Vector2f size() const
    {
        return mve::Vector2f(m_patch.size() * m_patch.scale());
    }

    [[nodiscard]] mve::Vector2f position() const
    {
        return m_position;
    }

    [[nodiscard]] float scale() const
    {
        return m_scale;
    }

    [[nodiscard]] bool is_pressed() const
    {
        return m_state == State::pressed;
    }

    [[nodiscard]] bool is_down() const
    {
        return m_state == State::down;
    }

    [[nodiscard]] bool is_released() const
    {
        return m_prev_state == State::down && m_state != State::down;
    }

    [[nodiscard]] bool is_hovering() const
    {
        return m_state == State::hover;
    }

private:
    [[nodiscard]] bool is_pos_in_button(const mve::Vector2f& pos) const;

    enum class State { none, hover, pressed, down };
    TextPipeline* m_text_pipeline;
    NinePatch m_patch;
    TextBuffer m_text;
    mve::Vector2f m_position;
    std::shared_ptr<mve::Texture> m_texture;
    std::shared_ptr<mve::Texture> m_texture_pressed;
    std::shared_ptr<mve::Texture> m_texture_hover;
    State m_prev_state;
    State m_state;
    float m_scale;
};