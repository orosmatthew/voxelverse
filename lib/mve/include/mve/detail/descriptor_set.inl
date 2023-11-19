#pragma once

#include "defs.hpp"

namespace mve {

inline DescriptorSet::DescriptorSet(
    Renderer& renderer, const GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set)
{
    *this = std::move(renderer.create_descriptor_set(graphics_pipeline, descriptor_set));
}

inline DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}

inline DescriptorSet::~DescriptorSet()
{
    destroy();
}

inline void DescriptorSet::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}

inline DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}

inline bool DescriptorSet::operator==(const DescriptorSet& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
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
    return m_renderer != nullptr;
}
inline void DescriptorSet::write_binding(
    const ShaderDescriptorBinding& descriptor_binding, const UniformBuffer& uniform_buffer) const
{
    m_renderer->write_descriptor_binding(*this, descriptor_binding, uniform_buffer);
}
inline void DescriptorSet::write_binding(
    const ShaderDescriptorBinding& descriptor_binding, const Texture& texture) const
{
    m_renderer->write_descriptor_binding(*this, descriptor_binding, texture);
}

inline DescriptorSet::DescriptorSet(Renderer& renderer, const size_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}
inline void DescriptorSet::invalidate()
{
    m_renderer = nullptr;
}

}

template <>
struct std::hash<mve::DescriptorSet> {
    std::size_t operator()(const mve::DescriptorSet& descriptor_set) const noexcept
    {
        return hash<uint64_t>()(descriptor_set.handle());
    }
};