#pragma once

#include <filesystem>
#include <functional>

#include <nnm/nnm.hpp>

#include <mve/vertex_data.hpp>

namespace mve {

using Handle = size_t;

class Renderer;
class ShaderDescriptorSet;
class ShaderDescriptorBinding;
class Shader;
class UniformLocation;

class DescriptorSet;
class Framebuffer;
class GraphicsPipeline;
class IndexBuffer;
class Monitor;
class Texture;
class UniformBuffer;
class VertexBuffer;

class DescriptorSet {

public:
    inline DescriptorSet(
        Renderer& renderer, const GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set);

    inline DescriptorSet(Renderer& renderer, Handle handle);

    DescriptorSet(const DescriptorSet&) = delete;

    inline DescriptorSet(DescriptorSet&& other) noexcept;

    inline ~DescriptorSet();

    inline void destroy();

    DescriptorSet& operator=(const DescriptorSet&) = delete;

    inline DescriptorSet& operator=(DescriptorSet&& other) noexcept;

    [[nodiscard]] inline bool operator==(const DescriptorSet& other) const;

    [[nodiscard]] inline bool operator<(const DescriptorSet& other) const;

    [[nodiscard]] inline Handle handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

    inline void write_binding(const ShaderDescriptorBinding& descriptor_binding, const UniformBuffer& uniform_buffer);

    inline void write_binding(const ShaderDescriptorBinding& descriptor_binding, const Texture& texture);

private:
    Renderer* m_renderer {};
    Handle m_handle {};
};

class Framebuffer {
public:
    inline explicit Framebuffer(Renderer& renderer);

    inline Framebuffer(Renderer& renderer, Handle handle);

    Framebuffer(const Framebuffer&) = delete;

    inline Framebuffer(Framebuffer&& other) noexcept;

    inline ~Framebuffer();

    inline void set_resize_callback(std::function<void()> callback);

    inline void remove_resize_callback();

    inline void destroy();

    [[nodiscard]] inline const Texture& texture() const;

    Framebuffer& operator=(const Framebuffer&) = delete;

    inline Framebuffer& operator=(Framebuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const Framebuffer& other) const;

    [[nodiscard]] inline bool operator<(const Framebuffer&& other) const;

    [[nodiscard]] inline Handle handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    Renderer* m_renderer {};
    Handle m_handle {};
};

class GraphicsPipeline {
public:
    inline GraphicsPipeline(
        Renderer& renderer,
        const Shader& vertex_shader,
        const Shader& fragment_shader,
        std::span<const VertexAttributeType> vertex_layout);

    inline GraphicsPipeline(Renderer& renderer, Handle handle);

    GraphicsPipeline(const GraphicsPipeline&) = delete;

    inline GraphicsPipeline(GraphicsPipeline&& other) noexcept;

    inline ~GraphicsPipeline();

    inline void destroy();

    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    inline GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

    [[nodiscard]] inline bool operator==(const GraphicsPipeline& other) const;

    [[nodiscard]] inline bool operator<(const GraphicsPipeline& other) const;

    [[nodiscard]] inline Handle handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

    [[nodiscard]] inline DescriptorSet create_descriptor_set(const ShaderDescriptorSet& descriptor_set) const;

private:
    Renderer* m_renderer {};
    Handle m_handle {};
};

class IndexBuffer {
public:
    IndexBuffer() = default;

    inline IndexBuffer(Renderer& renderer, std::span<uint32_t> indices);

    inline IndexBuffer(Renderer& renderer, Handle handle);

    IndexBuffer(const IndexBuffer&) = delete;

    inline IndexBuffer(IndexBuffer&& other) noexcept;

    inline ~IndexBuffer();

    inline void destroy();

    IndexBuffer& operator=(const IndexBuffer&) = delete;

    inline IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const IndexBuffer& other) const;

    [[nodiscard]] inline bool operator<(const IndexBuffer& other) const;

    [[nodiscard]] inline Handle handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    Renderer* m_renderer {};
    Handle m_handle {};
};

class Texture {
public:
    Texture() = default;

    inline Texture(Renderer& renderer, const std::filesystem::path& path);

    inline Texture(Renderer& renderer, Handle handle);

    Texture(const Texture&) = delete;

    inline Texture(Texture&& other) noexcept;

    inline ~Texture();

    inline void destroy();

    [[nodiscard]] inline nnm::Vector2i size() const;

    Texture& operator=(const Texture&) = delete;

    inline Texture& operator=(Texture&& other) noexcept;

    [[nodiscard]] inline bool operator==(const Texture& other) const;

    [[nodiscard]] inline bool operator<(const Texture& other) const;

    [[nodiscard]] inline Handle handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    Renderer* m_renderer {};
    Handle m_handle {};
};

class UniformBuffer {
public:
    inline UniformBuffer(Renderer& renderer, const ShaderDescriptorBinding& descriptor_binding);

    inline UniformBuffer(Renderer& renderer, Handle handle);

    UniformBuffer(const UniformBuffer&) = delete;

    inline UniformBuffer(UniformBuffer&& other) noexcept;

    inline ~UniformBuffer();

    inline void destroy();

    UniformBuffer& operator=(const UniformBuffer&) = delete;

    inline UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const UniformBuffer& other) const;

    [[nodiscard]] inline bool operator<(const UniformBuffer& other) const;

    [[nodiscard]] inline Handle handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

    inline void update(UniformLocation location, float value, bool persist = true);

    inline void update(UniformLocation location, nnm::Vector2f value, bool persist = true);

    inline void update(UniformLocation location, nnm::Vector3f value, bool persist = true);

    inline void update(UniformLocation location, nnm::Vector4f value, bool persist = true);

    inline void update(UniformLocation location, const nnm::Matrix3f& value, bool persist = true);

    inline void update(UniformLocation location, const nnm::Matrix4f& value, bool persist = true);

    inline void update(UniformLocation location, bool value, bool persist = true);

private:
    Renderer* m_renderer {};
    Handle m_handle {};
};

class VertexBuffer {
public:
    VertexBuffer() = default;

    inline VertexBuffer(Renderer& renderer, const VertexData& vertex_data);

    inline VertexBuffer(Renderer& renderer, Handle handle);

    VertexBuffer(const VertexBuffer&) = delete;

    inline VertexBuffer(VertexBuffer&& other) noexcept;

    inline ~VertexBuffer();

    inline void destroy();

    VertexBuffer& operator=(const VertexBuffer&) = delete;

    inline VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const VertexBuffer& other) const;

    [[nodiscard]] inline bool operator<(const VertexBuffer& other) const;

    [[nodiscard]] inline Handle handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    Renderer* m_renderer {};
    Handle m_handle {};
};

}
