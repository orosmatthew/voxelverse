#pragma once

#include "text_pipeline.hpp"

class TextBuffer {

    friend TextPipeline;

public:
    TextBuffer() = default;

    explicit TextBuffer(TextPipeline& pipeline)
    {
        *this = std::move(pipeline.create_text_buffer());
    }

    TextBuffer(
        TextPipeline& pipeline,
        const std::string_view text,
        const mve::Vector2f pos,
        const float scale,
        const mve::Vector3f color)
    {
        *this = std::move(pipeline.create_text_buffer(text, pos, scale, color));
    }

    TextBuffer(TextBuffer&& other) noexcept
        : m_valid(other.m_valid)
        , m_pipeline(other.m_pipeline)
        , m_handle(other.m_handle)
    {
        other.m_valid = false;
    }

    ~TextBuffer()
    {
        if (m_valid) {
            m_pipeline->destroy(*this);
        }
    }

    [[nodiscard]] size_t handle() const
    {
        return m_handle;
    }

    [[nodiscard]] bool is_valid() const
    {
        return m_valid;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void update(const std::string_view text)
    {
        m_pipeline->update_text_buffer(*this, text);
    }

    void set_translation(const mve::Vector2f pos) const
    {
        m_pipeline->set_text_buffer_translation(*this, pos);
    }

    void set_color(const mve::Vector3f color) const
    {
        m_pipeline->set_text_buffer_color(*this, color);
    }

    void set_scale(const float scale) const
    {
        m_pipeline->set_text_buffer_scale(*this, scale);
    }

    [[nodiscard]] float width() const
    {
        return m_pipeline->text_buffer_width(*this);
    }

    void add_cursor(const int pos) const
    {
        m_pipeline->add_cursor(*this, pos);
    }

    void set_cursor_pos(const int pos) const
    {
        m_pipeline->set_cursor_pos(*this, pos);
    }

    void cursor_left() const
    {
        m_pipeline->cursor_left(*this);
    }

    void cursor_right() const
    {
        m_pipeline->cursor_right(*this);
    }

    void remove_cursor() const
    {
        m_pipeline->remove_cursor(*this);
    }

    [[nodiscard]] std::optional<int> cursor_pos() const
    {
        return m_pipeline->cursor_pos(*this);
    }

    void draw() const
    {
        m_pipeline->draw(*this);
    }

    TextBuffer& operator=(TextBuffer&& other) noexcept
    {
        if (m_valid) {
            m_pipeline->destroy(*this);
        }
        m_valid = other.m_valid;
        m_pipeline = other.m_pipeline;
        m_handle = other.m_handle;
        other.m_valid = false;
        return *this;
    }

    bool operator==(const TextBuffer& other) const
    {
        return m_valid == other.m_valid && m_pipeline == other.m_pipeline && m_handle == other.m_handle;
    }

    bool operator<(const TextBuffer& other) const
    {
        return m_handle < other.m_handle;
    }

private:
    bool m_valid = false;
    TextPipeline* m_pipeline {};
    size_t m_handle {};
};

template <>
struct std::hash<TextBuffer> {
    std::size_t operator()(const TextBuffer& text_buffer) const noexcept
    {
        return hash<size_t>()(text_buffer.handle());
    }
};