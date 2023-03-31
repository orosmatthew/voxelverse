#include "button.hpp"

Button::Button(
    UIPipeline& ui_pipeline,
    TextPipeline& text_pipeline,
    std::shared_ptr<mve::Texture> texture,
    const std::string& text,
    float scale)
    : m_text_pipeline(&text_pipeline)
    , m_patch(ui_pipeline, texture, { 2, 2, 2, 2 }, { 30, 20 }, scale)
    , m_text(text_pipeline, text, { 0, 0 }, 1.0f, { 0.0f, 0.0f, 0.0f })
    , m_position(mve::Vector2(0.0f, 0.0f))
    , m_texture(texture)
    , m_scale(scale)
{
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
        { (pos.x + (m_patch.size().x * m_patch.scale()) / 2.0f) - m_text.width() / 2.0f,
          (pos.y + (m_patch.size().y * m_patch.scale()) / 2.0f) - m_text_pipeline->point_size() / 2.0f });
}
void Button::set_scale(float scale)
{
    m_patch.set_scale(scale);
    m_text.set_scale(scale);
}
