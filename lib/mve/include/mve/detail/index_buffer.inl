#pragma once

#include "defs.hpp"

namespace mve {

inline IndexBuffer::IndexBuffer(Renderer& renderer, const std::vector<uint32_t>& indices)
{
    *this = std::move(renderer.create_index_buffer(indices));
}

inline IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : m_handle(other.m_handle)
    , m_renderer(other.m_renderer)
{
    other.m_renderer = nullptr;
}

inline IndexBuffer::~IndexBuffer()
{
    destroy();
}
inline void IndexBuffer::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}

inline size_t IndexBuffer::handle() const
{
    return m_handle;
}

inline bool IndexBuffer::is_valid() const
{
    return m_renderer != nullptr;
}

inline IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}

inline bool IndexBuffer::operator==(const IndexBuffer& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
}
inline bool IndexBuffer::operator<(const IndexBuffer& other) const
{
    return m_handle < other.m_handle;
}
inline IndexBuffer::IndexBuffer(Renderer& renderer, const size_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}
inline void IndexBuffer::invalidate()
{
    m_renderer = nullptr;
}

}

template <>
struct std::hash<mve::IndexBuffer> {
    std::size_t operator()(const mve::IndexBuffer& index_buffer) const noexcept
    {
        return hash<size_t>()(index_buffer.handle());
    }
};