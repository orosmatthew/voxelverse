#include "console.hpp"

#include "../mve/window.hpp"

Console::Console(TextPipeline& pipeline)
    : m_input_text(pipeline)
{
    m_input_text.add_cursor(0);
}

void Console::resize(mve::Vector2i extent)
{
    m_extent = extent;
    float x = -extent.x / 2.0f + 8.0f;
    float y = -extent.y / 2.0f + 105.0f;
    m_input_text.set_translation({ x, y });
}

void Console::draw() const
{
    m_input_text.draw();
}

void Console::input_char(char character)
{
    m_input_str.insert(m_input_str.cbegin() + m_input_text.cursor_pos(), character);
    m_input_text.update(m_input_str);
    m_input_text.cursor_right();
}

void Console::backspace()
{
    if (m_input_str.empty() || m_input_text.cursor_pos() == 0) {
        return;
    }
    m_input_str.erase(m_input_str.cbegin() + (m_input_text.cursor_pos() - 1));
    m_input_text.cursor_left();
    m_input_text.update(m_input_str);
}

void Console::update_from_window(const mve::Window& window)
{
    for (const std::string& str : window.input_stream()) {
        for (auto c = str.begin(); c != str.end(); c++) {
            input_char(*c);
        }
    }
    if (window.is_key_pressed(mve::Key::backspace) || window.is_key_repeated(mve::Key::backspace)) {
        backspace();
    }
    if (window.is_key_pressed(mve::Key::left) || window.is_key_repeated(mve::Key::left)) {
        int new_pos = m_input_text.cursor_pos() - 1;
        new_pos = std::clamp(new_pos, 0, static_cast<int>(m_input_str.length()));
        m_input_text.set_cursor_pos(new_pos);
    }
    if (window.is_key_pressed(mve::Key::right) || window.is_key_repeated(mve::Key::right)) {
        int new_pos = m_input_text.cursor_pos() + 1;
        new_pos = std::clamp(new_pos, 0, static_cast<int>(m_input_str.length()));
        m_input_text.set_cursor_pos(new_pos);
    }
}
