#include "defs.hpp"

namespace mve {

inline Texture::Texture(Renderer& renderer, const std::filesystem::path& path)
{
    *this = std::move(renderer.create_texture(path));
}

inline Texture::Texture(Texture&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}
inline Texture::~Texture()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}
mve::Vector2i Texture::size() const
{
    return m_renderer->texture_size(*this);
}
inline Texture& Texture::operator=(Texture&& other)
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
inline bool Texture::operator==(const Texture& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
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
    return m_valid;
}

inline Texture::Texture(Renderer& renderer, uint64_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
inline void Texture::invalidate()
{
    m_valid = false;
}
inline Texture::Texture()
    : m_valid(false)
{
}
}

namespace std {
template <>
struct hash<mve::Texture> {
    std::size_t operator()(const mve::Texture& texture) const
    {
        return hash<uint64_t>()(texture.handle());
    }
};
}