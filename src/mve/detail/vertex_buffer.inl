#include "defs.hpp"

namespace mve {

inline VertexBuffer::VertexBuffer(Renderer& renderer, const VertexData& vertex_data)
{
    *this = std::move(renderer.create_vertex_buffer(vertex_data));
}

inline VertexBuffer::VertexBuffer(VertexBuffer&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}

inline VertexBuffer::~VertexBuffer()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}
inline size_t VertexBuffer::handle() const
{
    return m_handle;
}
inline bool VertexBuffer::is_valid() const
{
    return m_valid;
}

inline VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other)
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }

    m_valid = other.m_valid;
    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_valid = false;

    return *this;
}
inline bool VertexBuffer::operator==(const VertexBuffer& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
inline bool VertexBuffer::operator<(const VertexBuffer& other) const
{
    return m_handle < other.m_handle;
}

inline VertexBuffer::VertexBuffer(Renderer& renderer, size_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
inline void VertexBuffer::invalidate()
{
    m_valid = false;
}
inline VertexBuffer::VertexBuffer()
    : m_valid(false)
{
}
}

namespace std {

template <>
struct hash<mve::VertexBuffer> {
    std::size_t operator()(const mve::VertexBuffer& vertex_buffer) const
    {
        return hash<uint64_t>()(vertex_buffer.handle());
    }
};
}