#include "uniform_buffer.hpp"

#include "math/matrix4.hpp"
#include "renderer.hpp"

namespace mve {

UniformBuffer::UniformBuffer(Renderer& renderer, const ShaderDescriptorBinding& descriptor_binding)
{
    *this = renderer.create_uniform_buffer(descriptor_binding);
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
    m_renderer->destroy(*this);
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other)
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
bool UniformBuffer::operator==(const UniformBuffer& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
bool UniformBuffer::operator<(const UniformBuffer& other) const
{
    return m_handle < other.m_handle;
}
uint64_t UniformBuffer::handle() const
{
    return m_handle;
}
bool UniformBuffer::is_valid() const
{
    return m_valid;
}
void UniformBuffer::update(UniformLocation location, mve::Matrix4 value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
void UniformBuffer::update(UniformLocation location, mve::Matrix3 value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}

void UniformBuffer::update(UniformLocation location, mve::Vector4 value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
void UniformBuffer::update(UniformLocation location, mve::Vector3 value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
void UniformBuffer::update(UniformLocation location, mve::Vector2 value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
void UniformBuffer::update(UniformLocation location, float value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
void UniformBuffer::invalidate()
{
    m_valid = false;
}

UniformBuffer::UniformBuffer(Renderer& renderer, uint64_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}

}