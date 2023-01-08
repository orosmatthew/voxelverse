#include "texture.hpp"

#include "renderer.hpp"

namespace mve {

TextureHandle::TextureHandle()
    : m_initialized(false)
{
}
TextureHandle::TextureHandle(uint32_t value)
    : m_initialized(true)
    , m_value(value)
{
}
void TextureHandle::set(uint32_t value)
{
    m_initialized = true;
    m_value = value;
}
uint32_t TextureHandle::value() const
{
    return m_value;
}
bool TextureHandle::operator==(const TextureHandle& other) const
{
    return m_value == other.m_value;
}
bool TextureHandle::operator<(const TextureHandle& other) const
{
    return m_value < other.m_value;
}

Texture::Texture(Renderer& renderer, const std::filesystem::path& path)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(renderer.create_texture_handle(path))
{
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
    m_renderer->queue_destroy(m_handle);
}
Texture& Texture::operator=(Texture&& other)
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
bool Texture::operator==(const Texture& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
bool Texture::operator<(const Texture& other) const
{
    return m_handle < other.m_handle;
}
TextureHandle Texture::handle() const
{
    return m_handle;
}
bool Texture::is_valid() const
{
    return m_valid;
}

Texture::Texture(Renderer& renderer, TextureHandle handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
}