#pragma once

#include "defs.hpp"

namespace mve {

inline UniformBuffer::UniformBuffer(Renderer& renderer, const ShaderDescriptorBinding& descriptor_binding)
{
    *this = renderer.create_uniform_buffer(descriptor_binding);
}

inline UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}

inline UniformBuffer::~UniformBuffer()
{
    destroy();
}
inline void UniformBuffer::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}

inline UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}
inline bool UniformBuffer::operator==(const UniformBuffer& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
}
inline bool UniformBuffer::operator<(const UniformBuffer& other) const
{
    return m_handle < other.m_handle;
}
inline size_t UniformBuffer::handle() const
{
    return m_handle;
}
inline bool UniformBuffer::is_valid() const
{
    return m_renderer != nullptr;
}
inline void UniformBuffer::update(const UniformLocation location, const Matrix4& value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
inline void UniformBuffer::update(const UniformLocation location, const Matrix3& value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}

inline void UniformBuffer::update(const UniformLocation location, const Vector4 value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
inline void UniformBuffer::update(const UniformLocation location, const Vector3 value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
inline void UniformBuffer::update(const UniformLocation location, const Vector2 value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
inline void UniformBuffer::update(const UniformLocation location, const float value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
inline void UniformBuffer::invalidate()
{
    m_renderer = nullptr;
}

UniformBuffer::UniformBuffer(Renderer& renderer, const size_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}

}

template <>
struct std::hash<mve::UniformBuffer> {
    std::size_t operator()(const mve::UniformBuffer& uniform_buffer) const noexcept
    {
        return hash<size_t>()(uniform_buffer.handle());
    }
};