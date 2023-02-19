#include "defs.hpp"

namespace mve {

inline UniformBuffer::UniformBuffer(Renderer& renderer, const ShaderDescriptorBinding& descriptor_binding)
{
    *this = renderer.create_uniform_buffer(descriptor_binding);
}

inline UniformBuffer::UniformBuffer(UniformBuffer&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}

inline UniformBuffer::~UniformBuffer()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}

inline UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other)
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
inline bool UniformBuffer::operator==(const UniformBuffer& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
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
    return m_valid;
}
inline void UniformBuffer::update(UniformLocation location, mve::Matrix4 value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
inline void UniformBuffer::update(UniformLocation location, mve::Matrix3 value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}

inline void UniformBuffer::update(UniformLocation location, mve::Vector4 value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
inline void UniformBuffer::update(UniformLocation location, mve::Vector3 value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
inline void UniformBuffer::update(UniformLocation location, mve::Vector2 value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
inline void UniformBuffer::update(UniformLocation location, float value, bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}
inline void UniformBuffer::invalidate()
{
    m_valid = false;
}

UniformBuffer::UniformBuffer(Renderer& renderer, size_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}

}

namespace std {
template <>
struct hash<mve::UniformBuffer> {
    std::size_t operator()(const mve::UniformBuffer& uniform_buffer) const
    {
        return hash<size_t>()(uniform_buffer.handle());
    }
};
}