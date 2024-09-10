#include "console.hpp"

#include <mve/window.hpp>

Console::Console(TextPipeline& pipeline)
    : m_input_text(pipeline)
{
    m_input_text.set_color({ 1.0f, 1.0f, 1.0f });
}

void Console::resize(const nnm::Vector2i extent) const
{
    float x = 0.0f + 8.0f;
    float y = static_cast<float>(extent.y) - 150.0f;
    m_input_text.set_translation({ x, y });
    if (const std::optional<int> pos = m_input_text.cursor_pos(); pos.has_value()) {
        m_input_text.set_cursor_pos(*pos);
    }
}

void Console::draw() const
{
    m_input_text.draw();
}

void Console::input_char(const char character)
{
    const std::optional<int> pos = m_input_text.cursor_pos();
    if (pos.has_value()) {
        m_input_str.insert(m_input_str.cbegin() + *pos, character);
    }
    else {
        m_input_str.push_back(character);
    }
    m_input_text.update(m_input_str);
    if (pos.has_value()) {
        m_input_text.cursor_right();
    }
}

void Console::backspace()
{
    if (m_input_str.empty() || m_input_text.cursor_pos() == 0) {
        return;
    }
    if (const std::optional<int> pos = m_input_text.cursor_pos(); pos.has_value()) {
        m_input_str.erase(m_input_str.cbegin() + (*pos - 1));
        m_input_text.cursor_left();
    }
    else {
        m_input_str.pop_back();
    }
    m_input_text.update(m_input_str);
}

void Console::del()
{
    if (m_input_str.empty()) {
        return;
    }
    const std::optional<int> pos = m_input_text.cursor_pos();
    if (!pos.has_value()) {
        return;
    }
    m_input_str.erase(m_input_str.cbegin() + *pos);
    m_input_text.update(m_input_str);
}

void Console::update_from_window(const mve::Window& window)
{
    for (const char32_t c : window.input_char_stream()) {
        // TODO: Handle wide characters properly
        input_char(static_cast<char>(c));
    }
    if (window.is_key_pressed(mve::Key::backspace) || window.is_key_repeated(mve::Key::backspace)) {
        backspace();
    }
    if (window.is_key_pressed(mve::Key::del) || window.is_key_repeated(mve::Key::del)) {
        del();
    }
    const std::optional<int> pos = m_input_text.cursor_pos();
    if (pos.has_value() && window.is_key_pressed(mve::Key::left) || window.is_key_repeated(mve::Key::left)) {
        int new_pos = *pos - 1;
        new_pos = nnm::clamp(new_pos, 0, static_cast<int>(m_input_str.length()));
        m_input_text.set_cursor_pos(new_pos);
    }
    if (pos.has_value() && window.is_key_pressed(mve::Key::right) || window.is_key_repeated(mve::Key::right)) {
        int new_pos = *pos + 1;
        new_pos = nnm::clamp(new_pos, 0, static_cast<int>(m_input_str.length()));
        m_input_text.set_cursor_pos(new_pos);
    }
}
void Console::enable_cursor() const
{
    m_input_text.add_cursor(static_cast<int>(m_input_str.length()));
}
void Console::disable_cursor() const
{
    m_input_text.remove_cursor();
}
