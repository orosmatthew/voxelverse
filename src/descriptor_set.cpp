#include "descriptor_set.hpp"

#include "renderer.hpp"

namespace mve {

DescriptorSetHandle::DescriptorSetHandle()
    : m_initialized(false)
{
}
DescriptorSetHandle::DescriptorSetHandle(uint64_t value)
    : m_initialized(true)
    , m_value(value)
{
}

void DescriptorSetHandle::set(uint64_t value)
{
    m_initialized = true;
    m_value = value;
}
uint64_t DescriptorSetHandle::value() const
{
    return m_value;
}
bool DescriptorSetHandle::operator==(const DescriptorSetHandle& other) const
{
    return m_value == other.m_value && m_initialized == other.m_initialized;
}
bool DescriptorSetHandle::operator<(const DescriptorSetHandle& other) const
{
    return m_value < other.m_value;
}

DescriptorSet::DescriptorSet(
    Renderer& renderer, GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(renderer.create_descriptor_set_handle(graphics_pipeline.handle(), descriptor_set))
{
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
        m_renderer->queue_destroy(m_handle);
    }
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other)
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

bool DescriptorSet::operator==(const DescriptorSet& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
bool DescriptorSet::operator<(const DescriptorSet& other) const
{
    return m_handle < other.m_handle;
}
DescriptorSetHandle DescriptorSet::handle() const
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

DescriptorSet::DescriptorSet(Renderer& renderer, DescriptorSetHandle handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}

}