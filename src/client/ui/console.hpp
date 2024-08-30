#pragma once

#include "../text_buffer.hpp"
#include "../text_pipeline.hpp"

class Console {
public:
    explicit Console(TextPipeline& pipeline);

    void resize(nnm::Vector2i extent) const;

    void input_char(char character);

    void backspace();

    void del();

    void update_from_window(const mve::Window& window);

    void draw() const;

    void enable_cursor() const;

    void disable_cursor() const;

private:
    std::string m_input_str;
    TextBuffer m_input_text;
};