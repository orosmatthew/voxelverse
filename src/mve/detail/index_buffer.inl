#include "defs.hpp"

namespace mve {

inline IndexBuffer::IndexBuffer(Renderer& renderer, const std::vector<uint32_t>& indices)
{
    *this = std::move(renderer.create_index_buffer(indices));
}

inline IndexBuffer::IndexBuffer(IndexBuffer&& other)
    : m_valid(other.m_valid)
    , m_handle(other.m_handle)
    , m_renderer(other.m_renderer)
{
    other.m_valid = false;
}

inline IndexBuffer::~IndexBuffer()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}

inline size_t IndexBuffer::handle() const
{
    return m_handle;
}

inline bool IndexBuffer::is_valid() const
{
    return m_valid;
}

inline IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other)
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

inline bool IndexBuffer::operator==(const IndexBuffer& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
inline bool IndexBuffer::operator<(const IndexBuffer& other) const
{
    return m_handle < other.m_handle;
}
inline IndexBuffer::IndexBuffer(Renderer& renderer, size_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
inline void IndexBuffer::invalidate()
{
    m_valid = false;
}
IndexBuffer::IndexBuffer()
    : m_valid(false)
{
}
}

namespace std {
template <>
struct hash<mve::IndexBuffer> {
    std::size_t operator()(const mve::IndexBuffer& index_buffer) const
    {
        return hash<size_t>()(index_buffer.handle());
    }
};
}