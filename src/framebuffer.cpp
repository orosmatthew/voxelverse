#include "framebuffer.hpp"

#include "renderer.hpp"

namespace mve {

Framebuffer::Framebuffer(Renderer& renderer)
{
    *this = renderer.create_framebuffer();
}
Framebuffer::Framebuffer(Renderer& renderer, size_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
Framebuffer::Framebuffer(Framebuffer&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}
Framebuffer::~Framebuffer()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}
Framebuffer& Framebuffer::operator=(Framebuffer&& other)
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
bool Framebuffer::operator==(const Framebuffer& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
bool Framebuffer::operator<(const Framebuffer&& other) const
{
    return m_handle < other.m_handle;
}
size_t Framebuffer::handle() const
{
    return m_handle;
}
bool Framebuffer::is_valid() const
{
    return m_valid;
}
void Framebuffer::invalidate()
{
    m_valid = false;
}
const Texture& Framebuffer::texture() const
{
    return m_renderer->framebuffer_texture(*this);
}
}