#include "uniform_buffer.hpp"

#include "renderer.hpp"

namespace mve {

UniformBufferHandle::UniformBufferHandle()
    : m_initialized(false)
{
}
UniformBufferHandle::UniformBufferHandle(uint32_t value)
    : m_initialized(true)
    , m_value(value)
{
}
void UniformBufferHandle::set(uint32_t value)
{
    m_initialized = true;
    m_value = value;
}
uint32_t UniformBufferHandle::value() const
{
    return m_value;
}
bool UniformBufferHandle::operator==(const UniformBufferHandle& other) const
{
    return m_value == other.m_value;
}
bool UniformBufferHandle::operator<(const UniformBufferHandle& other) const
{
    return m_value < other.m_value;
}

UniformBuffer::UniformBuffer(Renderer& renderer, const ShaderDescriptorBinding& binding)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(renderer.create_uniform_buffer_handle(binding))
{
}

UniformBuffer::UniformBuffer(UniformBuffer&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}

UniformBuffer::~UniformBuffer()
{
    m_renderer->queue_destroy(m_handle);
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other)
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
bool UniformBuffer::operator==(const UniformBuffer& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
bool UniformBuffer::operator<(const UniformBuffer& other) const
{
    return m_handle < other.m_handle;
}
UniformBufferHandle UniformBuffer::handle() const
{
    return m_handle;
}
bool UniformBuffer::is_valid() const
{
    return m_valid;
}
void UniformBuffer::update(UniformLocation location, glm::mat4 value, bool persist)
{
    m_renderer->update_uniform(m_handle, location, value, persist);
}
void UniformBuffer::update(UniformLocation location, glm::mat3 value, bool persist)
{
    m_renderer->update_uniform(m_handle, location, value, persist);
}
void UniformBuffer::update(UniformLocation location, glm::mat2 value, bool persist)
{
    m_renderer->update_uniform(m_handle, location, value, persist);
}
void UniformBuffer::update(UniformLocation location, glm::vec4 value, bool persist)
{
    m_renderer->update_uniform(m_handle, location, value, persist);
}
void UniformBuffer::update(UniformLocation location, glm::vec3 value, bool persist)
{
    m_renderer->update_uniform(m_handle, location, value, persist);
}
void UniformBuffer::update(UniformLocation location, glm::vec2 value, bool persist)
{
    m_renderer->update_uniform(m_handle, location, value, persist);
}
void UniformBuffer::update(UniformLocation location, float value, bool persist)
{
    m_renderer->update_uniform(m_handle, location, value, persist);
}

}