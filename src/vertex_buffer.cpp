#include "vertex_buffer.hpp"

#include "renderer.hpp"
#include "vertex_data.hpp"

namespace mve {

VertexBuffer::VertexBuffer(Renderer& renderer, const VertexData& vertex_data)
{
    *this = std::move(renderer.create_vertex_buffer(vertex_data));
}

VertexBuffer::VertexBuffer(VertexBuffer&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}

VertexBuffer::~VertexBuffer()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}
size_t VertexBuffer::handle() const
{
    return m_handle;
}
bool VertexBuffer::is_valid() const
{
    return m_valid;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other)
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
bool VertexBuffer::operator==(const VertexBuffer& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
bool VertexBuffer::operator<(const VertexBuffer& other) const
{
    return m_handle < other.m_handle;
}

VertexBuffer::VertexBuffer(Renderer& renderer, size_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
void VertexBuffer::invalidate()
{
    m_valid = false;
}
}