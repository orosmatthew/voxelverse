#pragma once

#include "fwd.hpp"

namespace mve {

inline Framebuffer::Framebuffer(Renderer& renderer)
{
    *this = renderer.create_framebuffer();
}

inline Framebuffer::Framebuffer(Renderer& renderer, const size_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}

inline Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}

inline Framebuffer::~Framebuffer()
{
    destroy();
}

inline void Framebuffer::set_resize_callback(std::function<void()> callback)
{
    m_renderer->set_framebuffer_resize_callback(*this, std::move(callback));
}

inline void Framebuffer::remove_resize_callback()
{
    m_renderer->remove_framebuffer_resize_callback(*this);
}

inline void Framebuffer::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}

inline Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}

inline bool Framebuffer::operator==(const Framebuffer& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
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
    return m_renderer != nullptr;
}

inline void Framebuffer::invalidate()
{
    m_renderer = nullptr;
}

inline const Texture& Framebuffer::texture() const
{
    return m_renderer->framebuffer_texture(*this);
}
}

template <>
struct std::hash<mve::Framebuffer> {
    std::size_t operator()(const mve::Framebuffer& framebuffer) const noexcept
    {
        return hash<size_t>()(framebuffer.handle());
    }
};
