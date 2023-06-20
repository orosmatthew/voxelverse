#pragma once

#include "../text_buffer.hpp"
#include "nine_patch.hpp"

class Button {
public:
    explicit Button(
        UIPipeline& ui_pipeline,
        TextPipeline& text_pipeline,
        std::shared_ptr<mve::Texture> texture,
        const std::string& text,
        const mve::Vector2i& size,
        float scale = 1.0f);

    void draw() const;

    void set_position(const mve::Vector2& pos);

    void set_scale(float scale);

    void update(const mve::Window& window);

    void set_hover_texture(std::shared_ptr<mve::Texture> texture);

    void set_pressed_texture(std::shared_ptr<mve::Texture> texture);

    [[nodiscard]] inline mve::Vector2 size() const
    {
        return mve::Vector2(m_patch.size() * m_patch.scale());
    }

    [[nodiscard]] inline mve::Vector2 position() const
    {
        return m_position;
    }

    [[nodiscard]] inline float scale() const
    {
        return m_scale;
    }

    [[nodiscard]] inline bool is_pressed() const
    {
        return m_state == State::down && m_prev_state != State::down;
    }

    [[nodiscard]] inline bool is_down() const
    {
        return m_state == State::down;
    }

    [[nodiscard]] inline bool is_released() const
    {
        return m_prev_state == State::down && m_state != State::down;
    }

    [[nodiscard]] inline bool is_hovering() const
    {
        return m_state == State::hover;
    }

private:
    [[nodiscard]] bool is_pos_in_button(const mve::Vector2& pos) const;

    enum class State { none, hover, down };
    TextPipeline* m_text_pipeline;
    NinePatch m_patch;
    TextBuffer m_text;
    mve::Vector2 m_position;
    std::shared_ptr<mve::Texture> m_texture;
    std::shared_ptr<mve::Texture> m_texture_pressed;
    std::shared_ptr<mve::Texture> m_texture_hover;
    State m_prev_state;
    State m_state;
    float m_scale;
};