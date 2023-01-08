#include "graphics_pipeline.hpp"

#include "renderer.hpp"

namespace mve {

GraphicsPipelineHandle::GraphicsPipelineHandle(uint32_t value)
    : m_value(value)
    , m_initialized(true)
{
}
uint32_t GraphicsPipelineHandle::value() const
{
    return m_value;
}
bool GraphicsPipelineHandle::operator==(const GraphicsPipelineHandle& other) const
{
    return m_value == other.m_value;
}
bool GraphicsPipelineHandle::operator<(const GraphicsPipelineHandle& other) const
{
    return m_value < other.m_value;
}

GraphicsPipelineHandle::GraphicsPipelineHandle()
    : m_initialized(false)
{
}

void GraphicsPipelineHandle::set(uint32_t value)
{
    m_value = value;
    m_initialized = true;
}

GraphicsPipeline::GraphicsPipeline(
    Renderer& renderer, const Shader& vertex_shader, const Shader& fragment_shader, const VertexLayout& vertex_layout)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(renderer.create_graphics_pipeline_handle(vertex_shader, fragment_shader, vertex_layout))
{
}

GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other)
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
bool GraphicsPipeline::operator==(const GraphicsPipeline& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
bool GraphicsPipeline::operator<(const GraphicsPipeline& other) const
{
    return m_handle < other.m_handle;
}
GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}
GraphicsPipeline::~GraphicsPipeline()
{
    if (m_valid) {
        m_renderer->queue_destroy(m_handle);
    }
}
GraphicsPipelineHandle GraphicsPipeline::handle() const
{
    return m_handle;
}
bool GraphicsPipeline::is_valid() const
{
    return m_valid;
}

DescriptorSet GraphicsPipeline::create_descriptor_set(const ShaderDescriptorSet& descriptor_set)
{
    return m_renderer->create_descriptor_set(*this, descriptor_set);
}

}