#include "defs.hpp"

namespace mve {

inline Framebuffer::Framebuffer(Renderer& renderer, std::function<void(void)> callback)
{
    *this = renderer.create_framebuffer(callback);
}
inline Framebuffer::Framebuffer(Renderer& renderer, size_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
inline Framebuffer::Framebuffer(Framebuffer&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}
inline Framebuffer::~Framebuffer()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}
inline Framebuffer& Framebuffer::operator=(Framebuffer&& other)
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
inline bool Framebuffer::operator==(const Framebuffer& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
inline bool Framebuffer::operator<(const Framebuffer&& other) const
{
    return m_handle < other.m_handle;
}
inline size_t Framebuffer::handle() const
{
    return m_handle;
}
inline bool Framebuffer::is_valid() const
{
    return m_valid;
}
inline void Framebuffer::invalidate()
{
    m_valid = false;
}
inline const Texture& Framebuffer::texture() const
{
    return m_renderer->framebuffer_texture(*this);
}
}

namespace std {
template <>
struct hash<mve::Framebuffer> {
    std::size_t operator()(const mve::Framebuffer& framebuffer) const
    {
        return hash<size_t>()(framebuffer.handle());
    }
};
}
