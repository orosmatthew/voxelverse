#include "texture.hpp"

#include "renderer.hpp"

namespace mve {

Texture::Texture(Renderer& renderer, const std::filesystem::path& path)
{
    *this = std::move(renderer.create_texture(path));
}

Texture::Texture(Texture&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}
Texture::~Texture()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}
Texture& Texture::operator=(Texture&& other)
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
bool Texture::operator==(const Texture& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
bool Texture::operator<(const Texture& other) const
{
    return m_handle < other.m_handle;
}
uint64_t Texture::handle() const
{
    return m_handle;
}
bool Texture::is_valid() const
{
    return m_valid;
}

Texture::Texture(Renderer& renderer, uint64_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
void Texture::invalidate()
{
    m_valid = false;
}
}