#include "descriptor_set.hpp"

#include "renderer.hpp"

namespace mve {

DescriptorSet::DescriptorSet(
    Renderer& renderer, GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set)
{
    *this = std::move(renderer.create_descriptor_set(graphics_pipeline, descriptor_set));
}

DescriptorSet::DescriptorSet(DescriptorSet&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}

DescriptorSet::~DescriptorSet()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other)
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

bool DescriptorSet::operator==(const DescriptorSet& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
bool DescriptorSet::operator<(const DescriptorSet& other) const
{
    return m_handle < other.m_handle;
}
uint64_t DescriptorSet::handle() const
{
    return m_handle;
}
bool DescriptorSet::is_valid() const
{
    return m_valid;
}
void DescriptorSet::write_binding(const ShaderDescriptorBinding& descriptor_binding, UniformBuffer& uniform_buffer)
{
    m_renderer->write_descriptor_binding(*this, descriptor_binding, uniform_buffer);
}
void DescriptorSet::write_binding(const ShaderDescriptorBinding& descriptor_binding, Texture& texture)
{
    m_renderer->write_descriptor_binding(*this, descriptor_binding, texture);
}

DescriptorSet::DescriptorSet(Renderer& renderer, uint64_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}
void DescriptorSet::invalidate()
{
    m_valid = false;
}

}