#include "graphics_pipeline.hpp"

#include "renderer.hpp"

namespace mve {

GraphicsPipeline::GraphicsPipeline(
    Renderer& renderer, const Shader& vertex_shader, const Shader& fragment_shader, const VertexLayout& vertex_layout)
{
    *this = std::move(renderer.create_graphics_pipeline(vertex_shader, fragment_shader, vertex_layout));
}

GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other)
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
        m_renderer->destroy(*this);
    }
}
uint64_t GraphicsPipeline::handle() const
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
void GraphicsPipeline::invalidate()
{
    m_valid = false;
}

GraphicsPipeline::GraphicsPipeline(Renderer& renderer, uint64_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}

}