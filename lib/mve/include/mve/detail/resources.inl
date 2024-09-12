#pragma once

#include <mve/detail/resources.hpp>
#include <mve/shader.hpp>

namespace mve {

// DescriptorSet

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
    const ShaderDescriptorBinding& descriptor_binding, const UniformBuffer& uniform_buffer)
{
    m_renderer->write_descriptor_binding(*this, descriptor_binding, uniform_buffer);
}

inline void DescriptorSet::write_binding(const ShaderDescriptorBinding& descriptor_binding, const Texture& texture)
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

// Framebuffer

inline Framebuffer::Framebuffer(Renderer& renderer)
{
    *this = renderer.create_framebuffer();
}

inline Framebuffer::Framebuffer(Renderer& renderer, const size_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}

inline Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}

inline Framebuffer::~Framebuffer()
{
    destroy();
}

inline void Framebuffer::set_resize_callback(std::function<void()> callback)
{
    m_renderer->set_framebuffer_resize_callback(*this, std::move(callback));
}

inline void Framebuffer::remove_resize_callback()
{
    m_renderer->remove_framebuffer_resize_callback(*this);
}

inline void Framebuffer::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}

inline Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}

inline bool Framebuffer::operator==(const Framebuffer& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
}

inline bool Framebuffer::operator<(const Framebuffer&& other) const
{
    return m_handle < other.m_handle;
}

inline size_t Framebuffer::handle() const
{
    return m_handle;
}

inline bool Framebuffer::is_valid() const
{
    return m_renderer != nullptr;
}

inline void Framebuffer::invalidate()
{
    m_renderer = nullptr;
}

inline const Texture& Framebuffer::texture() const
{
    return m_renderer->framebuffer_texture(*this);
}

// GraphicsPipeline

inline GraphicsPipeline::GraphicsPipeline(
    Renderer& renderer,
    const Shader& vertex_shader,
    const Shader& fragment_shader,
    const std::span<const VertexAttributeType> vertex_layout)
{
    *this = std::move(renderer.create_graphics_pipeline(vertex_shader, fragment_shader, vertex_layout));
}

inline GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}

inline bool GraphicsPipeline::operator==(const GraphicsPipeline& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
}

inline bool GraphicsPipeline::operator<(const GraphicsPipeline& other) const
{
    return m_handle < other.m_handle;
}

inline GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}

inline GraphicsPipeline::~GraphicsPipeline()
{
    destroy();
}

inline void GraphicsPipeline::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}

inline size_t GraphicsPipeline::handle() const
{
    return m_handle;
}

inline bool GraphicsPipeline::is_valid() const
{
    return m_renderer != nullptr;
}

inline DescriptorSet GraphicsPipeline::create_descriptor_set(const ShaderDescriptorSet& descriptor_set) const
{
    return m_renderer->create_descriptor_set(*this, descriptor_set);
}

inline void GraphicsPipeline::invalidate()
{
    m_renderer = nullptr;
}

GraphicsPipeline::GraphicsPipeline(Renderer& renderer, const size_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}

// IndexBuffer

inline IndexBuffer::IndexBuffer(Renderer& renderer, const std::span<uint32_t> indices)
{
    *this = std::move(renderer.create_index_buffer(indices));
}

inline IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}

inline IndexBuffer::~IndexBuffer()
{
    destroy();
}

inline void IndexBuffer::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}

inline size_t IndexBuffer::handle() const
{
    return m_handle;
}

inline bool IndexBuffer::is_valid() const
{
    return m_renderer != nullptr;
}

inline IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}

inline bool IndexBuffer::operator==(const IndexBuffer& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
}

inline bool IndexBuffer::operator<(const IndexBuffer& other) const
{
    return m_handle < other.m_handle;
}

inline IndexBuffer::IndexBuffer(Renderer& renderer, const size_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}

inline void IndexBuffer::invalidate()
{
    m_renderer = nullptr;
}

// Texture

inline Texture::Texture(Renderer& renderer, const std::filesystem::path& path)
{
    *this = std::move(renderer.create_texture(path));
}

inline Texture::Texture(Texture&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}

inline Texture::~Texture()
{
    destroy();
}

inline void Texture::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}

nnm::Vector2i Texture::size() const
{
    return m_renderer->texture_size(*this);
}

inline Texture& Texture::operator=(Texture&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}

inline bool Texture::operator==(const Texture& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
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
    return m_renderer != nullptr;
}

inline Texture::Texture(Renderer& renderer, const uint64_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}

inline void Texture::invalidate()
{
    m_renderer = nullptr;
}

// UniformBuffer

inline UniformBuffer::UniformBuffer(Renderer& renderer, const ShaderDescriptorBinding& descriptor_binding)
{
    *this = renderer.create_uniform_buffer(descriptor_binding);
}

inline UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}

inline UniformBuffer::~UniformBuffer()
{
    destroy();
}

inline void UniformBuffer::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}

inline UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}

inline bool UniformBuffer::operator==(const UniformBuffer& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
}

inline bool UniformBuffer::operator<(const UniformBuffer& other) const
{
    return m_handle < other.m_handle;
}

inline size_t UniformBuffer::handle() const
{
    return m_handle;
}

inline bool UniformBuffer::is_valid() const
{
    return m_renderer != nullptr;
}

inline void UniformBuffer::update(const UniformLocation location, const nnm::Matrix4f& value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}

inline void UniformBuffer::update(const UniformLocation location, const bool value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}

inline void UniformBuffer::update(const UniformLocation location, const nnm::Matrix3f& value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}

inline void UniformBuffer::update(const UniformLocation location, const nnm::Vector4f value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}

inline void UniformBuffer::update(const UniformLocation location, const nnm::Vector3f value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}

inline void UniformBuffer::update(const UniformLocation location, const nnm::Vector2f value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}

inline void UniformBuffer::update(const UniformLocation location, const float value, const bool persist)
{
    m_renderer->update_uniform(*this, location, value, persist);
}

inline void UniformBuffer::invalidate()
{
    m_renderer = nullptr;
}

UniformBuffer::UniformBuffer(Renderer& renderer, const size_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}

// VertexBuffer

inline VertexBuffer::VertexBuffer(Renderer& renderer, const VertexData& vertex_data)
{
    *this = std::move(renderer.create_vertex_buffer(vertex_data));
}

inline VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_handle(other.m_handle)
{
    other.m_renderer = nullptr;
}

inline VertexBuffer::~VertexBuffer()
{
    destroy();
}

inline void VertexBuffer::destroy()
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }
}

inline size_t VertexBuffer::handle() const
{
    return m_handle;
}

inline bool VertexBuffer::is_valid() const
{
    return m_renderer != nullptr;
}

inline VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
    if (m_renderer != nullptr) {
        m_renderer->destroy(*this);
    }

    m_renderer = other.m_renderer;
    m_handle = other.m_handle;

    other.m_renderer = nullptr;

    return *this;
}

inline bool VertexBuffer::operator==(const VertexBuffer& other) const
{
    return m_renderer == other.m_renderer && m_handle == other.m_handle;
}

inline bool VertexBuffer::operator<(const VertexBuffer& other) const
{
    return m_handle < other.m_handle;
}

inline VertexBuffer::VertexBuffer(Renderer& renderer, const size_t handle)
    : m_renderer(&renderer)
    , m_handle(handle)
{
}

inline void VertexBuffer::invalidate()
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

template <>
struct std::hash<mve::Framebuffer> {
    std::size_t operator()(const mve::Framebuffer& framebuffer) const noexcept
    {
        return hash<size_t>()(framebuffer.handle());
    }
};

template <>
struct std::hash<mve::GraphicsPipeline> {
    std::size_t operator()(const mve::GraphicsPipeline& graphics_pipeline) const noexcept
    {
        return hash<uint64_t>()(graphics_pipeline.handle());
    }
};

template <>
struct std::hash<mve::IndexBuffer> {
    std::size_t operator()(const mve::IndexBuffer& index_buffer) const noexcept
    {
        return hash<size_t>()(index_buffer.handle());
    }
};

template <>
struct std::hash<mve::Texture> {
    std::size_t operator()(const mve::Texture& texture) const noexcept
    {
        return hash<uint64_t>()(texture.handle());
    }
};

template <>
struct std::hash<mve::UniformBuffer> {
    std::size_t operator()(const mve::UniformBuffer& uniform_buffer) const noexcept
    {
        return hash<size_t>()(uniform_buffer.handle());
    }
};

template <>
struct std::hash<mve::VertexBuffer> {
    std::size_t operator()(const mve::VertexBuffer& vertex_buffer) const noexcept
    {
        return hash<uint64_t>()(vertex_buffer.handle());
    }
};
