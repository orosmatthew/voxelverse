#include "descriptor_set.hpp"

#include "renderer.hpp"

namespace mve {

DescriptorSetHandle::DescriptorSetHandle()
    : m_initialized(false)
{
}
DescriptorSetHandle::DescriptorSetHandle(uint32_t value)
    : m_initialized(true)
    , m_value(value)
{
}

void DescriptorSetHandle::set(uint32_t value)
{
    m_initialized = true;
    m_value = value;
}
uint32_t DescriptorSetHandle::value() const
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

DescriptorSet::DescriptorSet(Renderer& renderer, GraphicsPipeline& pipeline, uint32_t set)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(renderer.create_descriptor_set_handle(pipeline, set))
{
}

DescriptorSet::DescriptorSet(Renderer& renderer, GraphicsPipelineHandle pipeline, uint32_t set)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(renderer.create_descriptor_set_handle(pipeline, set))
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
// void DescriptorSet::write_binding(const ShaderDescriptorBinding& binding, UniformBufferHandle uniform_buffer)
//{
//     m_renderer->write_descriptor_binding_uniform(m_handle, binding, uniform_buffer);
// }

}