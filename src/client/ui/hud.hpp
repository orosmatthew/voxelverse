#pragma once

#include "button.hpp"
#include "console.hpp"
#include "crosshair.hpp"
#include "debug_overlay.hpp"
#include "hotbar.hpp"

class HUD {
public:
    HUD(UIPipeline& ui_pipeline, TextPipeline& text_pipeline);

    void update_debug_player_block_pos(const mve::Vector3i pos)
    {
        m_debug_overlay.update_player_block_pos(pos);
    }

    void update_debug_fps(const int fps)
    {
        m_debug_overlay.update_fps(fps);
    }

    void update_debug_gpu_name(const std::string& gpu)
    {
        m_debug_overlay.update_gpu_name(gpu);
    }

    void update_console(const mve::Window& window)
    {
        m_console.update_from_window(window);
    }

    void resize(mve::Vector2i extent);

    void draw() const;

    void toggle_debug();

    [[nodiscard]] const Hotbar& hotbar() const
    {
        return m_hotbar;
    }

    Hotbar& hotbar()
    {
        return m_hotbar;
    }

    void enable_console_cursor() const
    {
        m_console.enable_cursor();
    }

    void disable_console_cursor() const
    {
        m_console.disable_cursor();
    }

    [[nodiscard]] bool is_debug_enabled() const
    {
        return m_show_debug;
    }

private:
    bool m_show_debug;
    Hotbar m_hotbar;
    Crosshair m_crosshair;
    DebugOverlay m_debug_overlay;
    Console m_console;
};