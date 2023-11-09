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
        Renderer& renderer, const GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set);

    inline DescriptorSet(Renderer& renderer, size_t handle);

    DescriptorSet(const DescriptorSet&) = delete;

    inline DescriptorSet(DescriptorSet&& other) noexcept;

    inline ~DescriptorSet();

    DescriptorSet& operator=(const DescriptorSet&) = delete;

    inline DescriptorSet& operator=(DescriptorSet&& other) noexcept;

    [[nodiscard]] inline bool operator==(const DescriptorSet& other) const;

    [[nodiscard]] inline bool operator<(const DescriptorSet& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

    inline void write_binding(
        const ShaderDescriptorBinding& descriptor_binding, const UniformBuffer& uniform_buffer) const;

    inline void write_binding(const ShaderDescriptorBinding& descriptor_binding, const Texture& texture) const;

private:
    bool m_valid = false;
    Renderer* m_renderer {};
    size_t m_handle {};
};

class Framebuffer {

    friend Renderer;

public:
    inline Framebuffer(Renderer& renderer, std::function<void()> callback);

    inline Framebuffer(Renderer& renderer, size_t handle);

    Framebuffer(const Framebuffer&) = delete;

    inline Framebuffer(Framebuffer&& other) noexcept;

    inline ~Framebuffer();

    [[nodiscard]] inline const Texture& texture() const;

    Framebuffer& operator=(const Framebuffer&) = delete;

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

    GraphicsPipeline(const GraphicsPipeline&) = delete;

    inline GraphicsPipeline(GraphicsPipeline&& other) noexcept;

    inline ~GraphicsPipeline();

    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    inline GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

    [[nodiscard]] inline bool operator==(const GraphicsPipeline& other) const;

    [[nodiscard]] inline bool operator<(const GraphicsPipeline& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

    [[nodiscard]] inline DescriptorSet create_descriptor_set(const ShaderDescriptorSet& descriptor_set) const;

private:
    bool m_valid = false;
    Renderer* m_renderer {};
    size_t m_handle {};
};

class IndexBuffer {

    friend Renderer;

public:
    IndexBuffer() = default;

    inline IndexBuffer(Renderer& renderer, const std::vector<uint32_t>& indices);

    inline IndexBuffer(Renderer& renderer, size_t handle);

    IndexBuffer(const IndexBuffer&) = delete;

    inline IndexBuffer(IndexBuffer&& other) noexcept;

    inline ~IndexBuffer();

    IndexBuffer& operator=(const IndexBuffer&) = delete;

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
    Texture() = default;

    inline Texture(Renderer& renderer, const std::filesystem::path& path);

    inline Texture(Renderer& renderer, uint64_t handle);

    Texture(const Texture&) = delete;

    inline Texture(Texture&& other) noexcept;

    inline ~Texture();

    [[nodiscard]] inline Vector2i size() const;

    Texture& operator=(const Texture&) = delete;

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

    friend Renderer;

public:
    inline UniformBuffer(Renderer& renderer, const ShaderDescriptorBinding& descriptor_binding);

    inline UniformBuffer(Renderer& renderer, size_t handle);

    UniformBuffer(const UniformBuffer&) = delete;

    inline UniformBuffer(UniformBuffer&& other) noexcept;

    inline ~UniformBuffer();

    UniformBuffer& operator=(const UniformBuffer&) = delete;

    inline UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const UniformBuffer& other) const;

    [[nodiscard]] inline bool operator<(const UniformBuffer& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

    inline void update(UniformLocation location, float value, bool persist = true);

    inline void update(UniformLocation location, Vector2 value, bool persist = true);

    inline void update(UniformLocation location, Vector3 value, bool persist = true);

    inline void update(UniformLocation location, Vector4 value, bool persist = true);

    inline void update(UniformLocation location, const Matrix3& value, bool persist = true);

    inline void update(UniformLocation location, const Matrix4& value, bool persist = true);

private:
    bool m_valid = false;
    Renderer* m_renderer {};
    size_t m_handle {};
};

class VertexBuffer {

    friend Renderer;

public:
    VertexBuffer() = default;

    inline VertexBuffer(Renderer& renderer, const VertexData& vertex_data);

    inline VertexBuffer(Renderer& renderer, size_t handle);

    VertexBuffer(const VertexBuffer&) = delete;

    inline VertexBuffer(VertexBuffer&& other) noexcept;

    inline ~VertexBuffer();

    VertexBuffer& operator=(const VertexBuffer&) = delete;

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
