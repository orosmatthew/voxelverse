#include "defs.hpp"

namespace mve {

inline GraphicsPipeline::GraphicsPipeline(
    Renderer& renderer, const Shader& vertex_shader, const Shader& fragment_shader, const VertexLayout& vertex_layout)
{
    *this = std::move(renderer.create_graphics_pipeline(vertex_shader, fragment_shader, vertex_layout));
}

inline GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other)
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
inline bool GraphicsPipeline::operator==(const GraphicsPipeline& other) const
{
    return m_valid == other.m_valid && m_renderer == other.m_renderer && m_handle == other.m_handle;
}
inline bool GraphicsPipeline::operator<(const GraphicsPipeline& other) const
{
    return m_handle < other.m_handle;
}
inline GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other)
    : m_valid(other.m_valid)
    , m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_valid = false;
}
inline GraphicsPipeline::~GraphicsPipeline()
{
    if (m_valid) {
        m_renderer->destroy(*this);
    }
}
inline size_t GraphicsPipeline::handle() const
{
    return m_handle;
}
inline bool GraphicsPipeline::is_valid() const
{
    return m_valid;
}

inline DescriptorSet GraphicsPipeline::create_descriptor_set(const ShaderDescriptorSet& descriptor_set)
{
    return m_renderer->create_descriptor_set(*this, descriptor_set);
}
inline void GraphicsPipeline::invalidate()
{
    m_valid = false;
}

GraphicsPipeline::GraphicsPipeline(Renderer& renderer, size_t handle)
    : m_valid(true)
    , m_renderer(&renderer)
    , m_handle(handle)
{
}

}

namespace std {
template <>
struct hash<mve::GraphicsPipeline> {
    std::size_t operator()(const mve::GraphicsPipeline& graphics_pipeline) const
    {
        return hash<uint64_t>()(graphics_pipeline.handle());
    }
};
}