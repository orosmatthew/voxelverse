#pragma once

#include "console.hpp"
#include "crosshair.hpp"
#include "debug_overlay.hpp"
#include "hotbar.hpp"

class HUD {
public:
    HUD(UIPipeline& ui_pipeline, TextPipeline& text_pipeline);

    inline void update_debug_player_block_pos(mve::Vector3i pos)
    {
        m_debug_overlay.update_player_block_pos(pos);
    }

    inline void update_debug_fps(int fps)
    {
        m_debug_overlay.update_fps(fps);
    }

    inline void update_debug_gpu_name(const std::string& gpu)
    {
        m_debug_overlay.update_gpu_name(gpu);
    }

    inline void update_console(const mve::Window& window)
    {
        m_console.update_from_window(window);
    }

    void resize(mve::Vector2i extent);

    void draw();

    void toggle_debug();

    inline const Hotbar& hotbar() const
    {
        return m_hotbar;
    }

    inline Hotbar& hotbar()
    {
        return m_hotbar;
    }

private:
    bool m_show_debug;
    Hotbar m_hotbar;
    Crosshair m_crosshair;
    DebugOverlay m_debug_overlay;
    Console m_console;
};