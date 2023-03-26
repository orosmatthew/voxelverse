#pragma once

#include <array>

#include "../text_buffer.hpp"

class DebugOverlay {
public:
    explicit DebugOverlay(TextPipeline& text_pipeline);

    void draw() const;

    void resize(mve::Vector2i extent);

    void update_fps(int value);

    void update_gpu_name(const std::string& gpu);

    void update_player_block_pos(mve::Vector3i pos);

private:
    mve::Vector2i m_extent;
    std::array<char, 1024> m_str_buffer;

    mve::Vector2 m_left_column_pos;
    float m_left_column_scale { 0.8f };
    float m_column_offset { 42.0f };
    std::vector<TextBuffer*> m_left_column {};

    TextBuffer m_fps_text;
    TextBuffer m_ms_text;
    TextBuffer m_gpu_text;
    TextBuffer m_build_text;
    TextBuffer m_player_block_text;
    TextBuffer m_player_chunk_text;
};