#pragma once

#include "../math/math.hpp"
#include <filesystem>
#include <functional>

#include "../vertex_data.hpp"

namespace mve {

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

    friend Renderer;

public:
    inline DescriptorSet(
        Renderer& renderer, GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set);

    inline DescriptorSet(Renderer& renderer, size_t handle);

    inline DescriptorSet(const DescriptorSet&) = delete;

    inline DescriptorSet(DescriptorSet&& other) noexcept;

    inline ~DescriptorSet();

    inline DescriptorSet& operator=(const DescriptorSet&) = delete;

    inline DescriptorSet& operator=(DescriptorSet&& other) noexcept;

    [[nodiscard]] inline bool operator==(const DescriptorSet& other) const;

    [[nodiscard]] inline bool operator<(const DescriptorSet& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

    inline void write_binding(const ShaderDescriptorBinding& descriptor_binding, UniformBuffer& uniform_buffer);

    inline void write_binding(const ShaderDescriptorBinding& descriptor_binding, const Texture& texture);

private:
    bool m_valid = false;
    Renderer* m_renderer {};
    size_t m_handle {};
};

class Framebuffer {

    friend Renderer;

public:
    inline Framebuffer(Renderer& renderer, std::function<void(void)> callback);

    inline Framebuffer(Renderer& renderer, size_t handle);

    inline Framebuffer(const Framebuffer&) = delete;

    inline Framebuffer(Framebuffer&& other) noexcept;

    inline ~Framebuffer();

    [[nodiscard]] inline const Texture& texture() const;

    inline Framebuffer& operator=(const Framebuffer&) = delete;

    inline Framebuffer& operator=(Framebuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const Framebuffer& other) const;

    [[nodiscard]] inline bool operator<(const Framebuffer&& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    bool m_valid = false;
    Renderer* m_renderer {};
    size_t m_handle {};
};

class GraphicsPipeline {
public:
    inline GraphicsPipeline(
        Renderer& renderer,
        const Shader& vertex_shader,
        const Shader& fragment_shader,
        const VertexLayout& vertex_layout);

    inline GraphicsPipeline(Renderer& renderer, size_t handle);

    inline GraphicsPipeline(const GraphicsPipeline&) = delete;

    inline GraphicsPipeline(GraphicsPipeline&& other) noexcept;

    inline ~GraphicsPipeline();

    inline GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    inline GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

    [[nodiscard]] inline bool operator==(const GraphicsPipeline& other) const;

    [[nodiscard]] inline bool operator<(const GraphicsPipeline& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

    inline DescriptorSet create_descriptor_set(const ShaderDescriptorSet& descriptor_set);

private:
    bool m_valid = false;
    Renderer* m_renderer {};
    size_t m_handle {};
};

class IndexBuffer {

    friend Renderer;

public:
    inline IndexBuffer();

    inline IndexBuffer(Renderer& renderer, const std::vector<uint32_t>& indices);

    inline IndexBuffer(Renderer& renderer, size_t handle);

    inline IndexBuffer(const IndexBuffer&) = delete;

    inline IndexBuffer(IndexBuffer&& other) noexcept;

    inline ~IndexBuffer();

    inline IndexBuffer& operator=(const IndexBuffer&) = delete;

    inline IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const IndexBuffer& other) const;

    [[nodiscard]] inline bool operator<(const IndexBuffer& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    bool m_valid = false;
    Renderer* m_renderer {};
    size_t m_handle {};
};

class Texture {

    friend Renderer;

public:
    inline Texture();

    inline Texture(Renderer& renderer, const std::filesystem::path& path);

    inline Texture(Renderer& renderer, uint64_t handle);

    inline Texture(const Texture&) = delete;

    inline Texture(Texture&& other) noexcept;

    inline ~Texture();

    [[nodiscard]] inline mve::Vector2i size() const;

    inline Texture& operator=(const Texture&) = delete;

    inline Texture& operator=(Texture&& other) noexcept;

    [[nodiscard]] inline bool operator==(const Texture& other) const;

    [[nodiscard]] inline bool operator<(const Texture& other) const;

    [[nodiscard]] inline uint64_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    bool m_valid = false;
    Renderer* m_renderer {};
    uint64_t m_handle {};
};

class UniformBuffer {

    friend mve::Renderer;

public:
    inline UniformBuffer(Renderer& renderer, const ShaderDescriptorBinding& descriptor_binding);

    inline UniformBuffer(Renderer& renderer, size_t handle);

    inline UniformBuffer(const UniformBuffer&) = delete;

    inline UniformBuffer(UniformBuffer&& other) noexcept;

    inline ~UniformBuffer();

    inline UniformBuffer& operator=(const UniformBuffer&) = delete;

    inline UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const UniformBuffer& other) const;

    [[nodiscard]] inline bool operator<(const UniformBuffer& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

    inline void update(UniformLocation location, float value, bool persist = true);

    inline void update(UniformLocation location, mve::Vector2 value, bool persist = true);

    inline void update(UniformLocation location, mve::Vector3 value, bool persist = true);

    inline void update(UniformLocation location, mve::Vector4 value, bool persist = true);

    inline void update(UniformLocation location, mve::Matrix3 value, bool persist = true);

    inline void update(UniformLocation location, mve::Matrix4 value, bool persist = true);

private:
    bool m_valid = false;
    Renderer* m_renderer {};
    size_t m_handle {};
};

class VertexBuffer {

    friend Renderer;

public:
    inline VertexBuffer();

    inline VertexBuffer(Renderer& renderer, const VertexData& vertex_data);

    inline VertexBuffer(Renderer& renderer, size_t handle);

    inline VertexBuffer(const VertexBuffer&) = delete;

    inline VertexBuffer(VertexBuffer&& other) noexcept;

    inline ~VertexBuffer();

    inline VertexBuffer& operator=(const VertexBuffer&) = delete;

    inline VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const VertexBuffer& other) const;

    [[nodiscard]] inline bool operator<(const VertexBuffer& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    bool m_valid = false;
    Renderer* m_renderer {};
    size_t m_handle {};
};

}
