#include "hud.hpp"

HUD::HUD(std::weak_ptr<UIPipeline> ui_pipeline, std::weak_ptr<TextPipeline> text_pipeline)
    : m_show_debug(false)
    , m_hotbar(*ui_pipeline.lock())
    , m_crosshair(*ui_pipeline.lock())
    , m_debug_overlay(*text_pipeline.lock())
    , m_console(*text_pipeline.lock())
    , m_button(
          ui_pipeline,
          text_pipeline,
          std::make_shared<mve::Texture>(ui_pipeline.lock()->renderer(), "../res/button_gray.png"),
          "Button Text",
          10.0f)
{
    m_button.set_position({ 10, 10 });

    m_hotbar.set_item(0, 1);
    m_hotbar.set_item(1, 2);
    m_hotbar.set_item(2, 3);
    m_hotbar.set_item(3, 4);
    m_hotbar.set_item(4, 5);
    m_hotbar.set_item(5, 6);
    m_hotbar.set_item(6, 7);
    m_hotbar.set_item(7, 8);
    m_hotbar.set_item(8, 9);
}
void HUD::resize(mve::Vector2i extent)
{
    m_hotbar.resize(extent);
    m_debug_overlay.resize(extent);
    m_console.resize(extent);
}

void HUD::draw()
{
    m_crosshair.draw();
    m_hotbar.draw();
    if (m_show_debug) {
        m_debug_overlay.draw();
    }
    m_console.draw();
    m_button.draw();
}

void HUD::toggle_debug()
{
    m_show_debug = !m_show_debug;
}
