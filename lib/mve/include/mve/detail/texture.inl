#pragma once

#include "defs.hpp"

namespace mve {

inline Texture::Texture(Renderer& renderer, const std::filesystem::path& path)
{
    *this = std::move(renderer.create_texture(path));
}

inline Texture::Texture(Texture&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}
inline Texture::~Texture()
{
    destroy();
}
inline void Texture::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}
nnm::Vector2i Texture::size() const
{
    return m_renderer->texture_size(*this);
}
inline Texture& Texture::operator=(Texture&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}
inline bool Texture::operator==(const Texture& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
}
inline bool Texture::operator<(const Texture& other) const
{
    return m_handle < other.m_handle;
}
inline uint64_t Texture::handle() const
{
    return m_handle;
}
inline bool Texture::is_valid() const
{
    return m_renderer != nullptr;
}

inline Texture::Texture(Renderer& renderer, const uint64_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}
inline void Texture::invalidate()
{
    m_renderer = nullptr;
}
}

template <>
struct std::hash<mve::Texture> {
    std::size_t operator()(const mve::Texture& texture) const noexcept
    {
        return hash<uint64_t>()(texture.handle());
    }
};