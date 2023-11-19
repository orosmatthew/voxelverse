#pragma once

#include "defs.hpp"

namespace mve {

inline VertexBuffer::VertexBuffer(Renderer& renderer, const VertexData& vertex_data)
{
    *this = std::move(renderer.create_vertex_buffer(vertex_data));
}

inline VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}

inline VertexBuffer::~VertexBuffer()
{
    destroy();
}
inline void VertexBuffer::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}
inline size_t VertexBuffer::handle() const
{
    return m_handle;
}
inline bool VertexBuffer::is_valid() const
{
    return m_renderer != nullptr;
}

inline VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}
inline bool VertexBuffer::operator==(const VertexBuffer& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
}
inline bool VertexBuffer::operator<(const VertexBuffer& other) const
{
    return m_handle < other.m_handle;
}

inline VertexBuffer::VertexBuffer(Renderer& renderer, const size_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}
inline void VertexBuffer::invalidate()
{
    m_renderer = nullptr;
}
}

template <>
struct std::hash<mve::VertexBuffer> {
    std::size_t operator()(const mve::VertexBuffer& vertex_buffer) const noexcept
    {
        return hash<uint64_t>()(vertex_buffer.handle());
    }
};