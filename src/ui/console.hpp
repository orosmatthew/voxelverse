#pragma once

#include "../text_buffer.hpp"
#include "../text_pipeline.hpp"

class Console {
public:
    explicit Console(TextPipeline& pipeline);

    void resize(mve::Vector2i extent);

    void input_char(char character);

    void backspace();

    void update_from_window(const mve::Window& window);

    void draw() const;

private:
    mve::Vector2i m_extent;
    std::string m_input_str;
    TextBuffer m_input_text;
};