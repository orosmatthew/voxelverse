#include "console.hpp"

#include "../mve/window.hpp"

Console::Console(TextPipeline& pipeline)
    : m_input_text(pipeline)
{
    m_input_text.update("", { 0.0f, 0.0f }, 0.8f);
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
    m_input_str.push_back(character);
    m_input_text.update(m_input_str, { 0.0f, 0.0f }, 0.8f);
    resize(m_extent);
}

void Console::backspace()
{
    if (m_input_str.empty()) {
        return;
    }
    m_input_str.pop_back();
    m_input_text.update(m_input_str, { 0.0f, 0.0f }, 0.8f);
    resize(m_extent);
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
}
