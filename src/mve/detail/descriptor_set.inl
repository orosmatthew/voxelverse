#include "defs.hpp"

namespace mve {

inline DescriptorSet::DescriptorSet(
    Renderer& renderer, GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set)
{
    *this = std::move(renderer.create_descriptor_set(graphics_pipeline, descriptor_set));
}

inline DescriptorSet::DescriptorSet(DescriptorSet&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}

inline DescriptorSet::~DescriptorSet()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}

inline DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other)
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

inline bool DescriptorSet::operator==(const DescriptorSet& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
inline bool DescriptorSet::operator<(const DescriptorSet& other) const
{
    return m_handle < other.m_handle;
}
inline size_t DescriptorSet::handle() const
{
    return m_handle;
}
inline bool DescriptorSet::is_valid() const
{
    return m_valid;
}
inline void DescriptorSet::write_binding(const ShaderDescriptorBinding& descriptor_binding, UniformBuffer& uniform_buffer)
{
    m_renderer->write_descriptor_binding(*this, descriptor_binding, uniform_buffer);
}
inline void DescriptorSet::write_binding(const ShaderDescriptorBinding& descriptor_binding, const Texture& texture)
{
    m_renderer->write_descriptor_binding(*this, descriptor_binding, texture);
}

inline DescriptorSet::DescriptorSet(Renderer& renderer, size_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
inline void DescriptorSet::invalidate()
{
    m_valid = false;
}

}

namespace std {

template <>
struct hash<mve::DescriptorSet> {
    std::size_t operator()(const mve::DescriptorSet& descriptor_set) const
    {
        return hash<uint64_t>()(descriptor_set.handle());
    }
};
}