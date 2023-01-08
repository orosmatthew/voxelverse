#include "index_buffer.hpp"

#include "renderer.hpp"

namespace mve {

IndexBuffer::IndexBuffer(Renderer& renderer, const std::vector<uint32_t>& indices)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(renderer.create_index_buffer_handle(indices))
{
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
    m_renderer->queue_destroy(m_handle);
}

IndexBufferHandle IndexBuffer::handle() const
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
        m_renderer->queue_destroy(m_handle);
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
IndexBuffer::IndexBuffer(Renderer& renderer, IndexBufferHandle handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}

IndexBufferHandle::IndexBufferHandle(uint64_t value)
    : m_initialized(true)
    , m_value(value)
{
}
uint64_t IndexBufferHandle::value() const
{
    return m_value;
}
bool IndexBufferHandle::operator==(const IndexBufferHandle& other) const
{
    return m_value == other.m_value;
}
bool IndexBufferHandle::operator<(const IndexBufferHandle& other) const
{
    return m_value < other.m_value;
}
IndexBufferHandle::IndexBufferHandle()
    : m_initialized(false)
{
}
void IndexBufferHandle::set(uint64_t value)
{
    m_initialized = true;
    m_value = value;
}
}