#include "button.hpp"

Button::Button(
    std::shared_ptr<UIPipeline> ui_pipeline,
    std::shared_ptr<TextPipeline> text_pipeline,
    std::shared_ptr<mve::Texture> texture,
    const std::string& text,
    float scale)
    : m_text_pipeline(text_pipeline)
    , m_patch(ui_pipeline, texture, { 2, 2, 2, 2 }, { 30, 20 }, scale)
    , m_text(*text_pipeline, text, { 0, 0 }, 1.0f, { 0.0f, 0.0f, 0.0f })
    , m_position(mve::Vector2(0.0f, 0.0f))
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
    auto text_pipeline_ref = m_text_pipeline.lock();
    MVE_VAL_ASSERT(text_pipeline_ref, "[Button] Invalid text pipeline")
    m_patch.set_position(pos);
    m_text.set_translation(
        { (pos.x + (m_patch.size().x * m_patch.scale()) / 2.0f) - m_text.width() / 2.0f,
          (pos.y + (m_patch.size().y * m_patch.scale()) / 2.0f) - text_pipeline_ref->point_size() / 2.0f });
}
void Button::set_scale(float scale)
{
    m_patch.set_scale(scale);
    m_text.set_scale(scale);
}
