#include "index_buffer.hpp"

#include "renderer.hpp"

namespace mve {

IndexBuffer::IndexBuffer(Renderer& renderer, const std::vector<uint32_t>& indices)
{
    *this = std::move(renderer.create_index_buffer(indices));
}

IndexBuffer::IndexBuffer(IndexBuffer&& other)
    : m_valid(other.m_valid)
    , m_handle(other.m_handle)
    , m_renderer(other.m_renderer)
{
    other.m_valid = false;
}

IndexBuffer::~IndexBuffer()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}

size_t IndexBuffer::handle() const
{
    return m_handle;
}

bool IndexBuffer::is_valid() const
{
    return m_valid;
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other)
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

bool IndexBuffer::operator==(const IndexBuffer& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
bool IndexBuffer::operator<(const IndexBuffer& other) const
{
    return m_handle < other.m_handle;
}
IndexBuffer::IndexBuffer(Renderer& renderer, size_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
void IndexBuffer::invalidate()
{
    m_valid = false;
}
}