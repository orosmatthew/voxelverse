#pragma once

#include "text_pipeline.hpp"

class TextBuffer {

    friend TextPipeline;

public:
    inline TextBuffer()
        : m_valid(false)
        , m_pipeline(nullptr)
        , m_handle(0)
    {
    }

    inline explicit TextBuffer(TextPipeline& pipeline)
    {
        *this = std::move(pipeline.create_text_buffer());
    }

    inline TextBuffer(TextBuffer&& other)
        : m_valid(other.m_valid)
        , m_pipeline(other.m_pipeline)
        , m_handle(other.m_handle)
    {
        other.m_valid = false;
    }

    inline ~TextBuffer()
    {
        if (m_valid) {
            m_pipeline->destroy(*this);
        }
    }

    inline size_t handle() const
    {
        return m_handle;
    }

    inline bool is_valid() const
    {
        return m_valid;
    }

    inline void update(std::string_view text, mve::Vector2 pos, float scale) const
    {
        m_pipeline->update_text_buffer(*this, text, pos, scale);
    }

    inline void set_translation(mve::Vector2 pos) const
    {
        m_pipeline->set_text_buffer_translation(*this, pos);
    }

    inline void draw() const
    {
        m_pipeline->draw(*this);
    }

    inline TextBuffer& operator=(TextBuffer&& other)
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

    inline bool operator==(const TextBuffer& other) const
    {
        return m_valid == other.m_valid && m_pipeline == other.m_pipeline && m_handle == other.m_handle;
    }

    inline bool operator<(const TextBuffer& other) const
    {
        return m_handle < other.m_handle;
    }

private:
    bool m_valid = false;
    TextPipeline* m_pipeline;
    size_t m_handle;
};

namespace std {
template <>
struct hash<TextBuffer> {
    std::size_t operator()(const TextBuffer& text_buffer) const
    {
        return hash<size_t>()(text_buffer.handle());
    }
};
}