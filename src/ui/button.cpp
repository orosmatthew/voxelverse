#include "button.hpp"

Button::Button(UIPipeline& pipeline, float scale)
    : m_patch(pipeline, "../res/button_gray.png", { 2, 2, 2, 2 }, { 30, 20 }, scale)
{
}

void Button::draw() const
{
    m_patch.draw();
}
