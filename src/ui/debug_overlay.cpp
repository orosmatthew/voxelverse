#include "debug_overlay.hpp"

#include "../logger.hpp"
#include "../world_data.hpp"

DebugOverlay::DebugOverlay(TextPipeline& text_pipeline)
    : m_fps_text(text_pipeline, "", { 0.0f, 0.0f }, 0.8f, c_text_color)
    , m_ms_text(text_pipeline, "", { 0.0f, 0.0f }, 0.8f, c_text_color)
    , m_gpu_text(text_pipeline, "", { 0.0f, 0.0f }, 0.8f, c_text_color)
    , m_build_text(text_pipeline, "", { 0.0f, 0.0f }, 0.8f, c_text_color)
    , m_player_block_text(text_pipeline, "", { 0.0f, 0.0f }, 0.8f, c_text_color)
    , m_player_chunk_text(text_pipeline, "", { 0.0f, 0.0f }, 0.8f, c_text_color)
{
    m_left_column.push_back(&m_fps_text);
    m_left_column.push_back(&m_ms_text);
    m_left_column.push_back(&m_gpu_text);
    m_left_column.push_back(&m_build_text);
    m_left_column.push_back(&m_player_block_text);
    m_left_column.push_back(&m_player_chunk_text);

#ifdef NDEBUG
    std::snprintf(m_str_buffer.data(), m_str_buffer.size(), "build: optimized");
#else
    std::snprintf(m_str_buffer.data(), m_str_buffer.size(), "build: debug");
#endif
    m_build_text.update(m_str_buffer.data());
    m_build_text.set_scale(0.8f);
}

void DebugOverlay::draw() const
{
    for (const TextBuffer* buffer : m_left_column) {
        buffer->draw();
    }
}

void DebugOverlay::update_fps(int value)
{
    std::snprintf(m_str_buffer.data(), m_str_buffer.size(), "fps: %d", value);
    m_fps_text.update(m_str_buffer.data());

    std::snprintf(m_str_buffer.data(), m_str_buffer.size(), "ms: %.1f", 1000.0f / value);
    m_ms_text.update(m_str_buffer.data());
}

void DebugOverlay::resize(mve::Vector2i extent)
{
    m_extent = extent;
    m_left_column_pos = { -m_extent.x / 2.0f + 8.0f * 1.0f, m_extent.y / 2.0f - 35.0f * 1.0f };
    float y = m_left_column_pos.y;
    for (TextBuffer* buffer : m_left_column) {
        buffer->set_translation({ m_left_column_pos.x, y });
        y -= m_column_offset * m_left_column_scale;
    }
}
void DebugOverlay::update_gpu_name(const std::string& gpu)
{
    std::snprintf(m_str_buffer.data(), m_str_buffer.size(), "gpu: %s", gpu.data());
    m_gpu_text.update(m_str_buffer.data());
}
void DebugOverlay::update_player_block_pos(mve::Vector3i pos)
{
    std::snprintf(m_str_buffer.data(), m_str_buffer.size(), "block: [%d, %d, %d]", pos.x, pos.y, pos.z);
    m_player_block_text.update(m_str_buffer.data());

    mve::Vector3i chunk_pos = WorldData::chunk_pos_from_block_pos(pos);
    std::snprintf(
        m_str_buffer.data(), m_str_buffer.size(), "chunk: [%d, %d, %d]", chunk_pos.x, chunk_pos.y, chunk_pos.z);
    m_player_chunk_text.update(m_str_buffer.data());
}
