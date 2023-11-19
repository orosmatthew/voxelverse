#pragma once

#include <filesystem>
#include <functional>
#include <queue>

#define VMA_VULKAN_VERSION 1001000
#include "vk_mem_alloc.h"
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include <mve/math/math.hpp>
#include <mve/vertex_data.hpp>

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

public:
    inline DescriptorSet(
        Renderer& renderer, const GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set);

    inline DescriptorSet(Renderer& renderer, size_t handle);

    DescriptorSet(const DescriptorSet&) = delete;

    inline DescriptorSet(DescriptorSet&& other) noexcept;

    inline ~DescriptorSet();

    inline void destroy();

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
    Renderer* m_renderer {};
    size_t m_handle {};
};

class Framebuffer {
public:
    inline Framebuffer(Renderer& renderer, std::function<void()> callback);

    inline Framebuffer(Renderer& renderer, size_t handle);

    Framebuffer(const Framebuffer&) = delete;

    inline Framebuffer(Framebuffer&& other) noexcept;

    inline ~Framebuffer();

    inline void destroy();

    [[nodiscard]] inline const Texture& texture() const;

    Framebuffer& operator=(const Framebuffer&) = delete;

    inline Framebuffer& operator=(Framebuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const Framebuffer& other) const;

    [[nodiscard]] inline bool operator<(const Framebuffer&& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
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

    inline void destroy();

    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    inline GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

    [[nodiscard]] inline bool operator==(const GraphicsPipeline& other) const;

    [[nodiscard]] inline bool operator<(const GraphicsPipeline& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

    [[nodiscard]] inline DescriptorSet create_descriptor_set(const ShaderDescriptorSet& descriptor_set) const;

private:
    Renderer* m_renderer {};
    size_t m_handle {};
};

class IndexBuffer {
public:
    IndexBuffer() = default;

    inline IndexBuffer(Renderer& renderer, const std::vector<uint32_t>& indices);

    inline IndexBuffer(Renderer& renderer, size_t handle);

    IndexBuffer(const IndexBuffer&) = delete;

    inline IndexBuffer(IndexBuffer&& other) noexcept;

    inline ~IndexBuffer();

    inline void destroy();

    IndexBuffer& operator=(const IndexBuffer&) = delete;

    inline IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const IndexBuffer& other) const;

    [[nodiscard]] inline bool operator<(const IndexBuffer& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    Renderer* m_renderer {};
    size_t m_handle {};
};

class Texture {
public:
    Texture() = default;

    inline Texture(Renderer& renderer, const std::filesystem::path& path);

    inline Texture(Renderer& renderer, uint64_t handle);

    Texture(const Texture&) = delete;

    inline Texture(Texture&& other) noexcept;

    inline ~Texture();

    inline void destroy();

    [[nodiscard]] inline Vector2i size() const;

    Texture& operator=(const Texture&) = delete;

    inline Texture& operator=(Texture&& other) noexcept;

    [[nodiscard]] inline bool operator==(const Texture& other) const;

    [[nodiscard]] inline bool operator<(const Texture& other) const;

    [[nodiscard]] inline uint64_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    Renderer* m_renderer {};
    uint64_t m_handle {};
};

class UniformBuffer {
public:
    inline UniformBuffer(Renderer& renderer, const ShaderDescriptorBinding& descriptor_binding);

    inline UniformBuffer(Renderer& renderer, size_t handle);

    UniformBuffer(const UniformBuffer&) = delete;

    inline UniformBuffer(UniformBuffer&& other) noexcept;

    inline ~UniformBuffer();

    inline void destroy();

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
    Renderer* m_renderer {};
    size_t m_handle {};
};

class VertexBuffer {
public:
    VertexBuffer() = default;

    inline VertexBuffer(Renderer& renderer, const VertexData& vertex_data);

    inline VertexBuffer(Renderer& renderer, size_t handle);

    VertexBuffer(const VertexBuffer&) = delete;

    inline VertexBuffer(VertexBuffer&& other) noexcept;

    inline ~VertexBuffer();

    inline void destroy();

    VertexBuffer& operator=(const VertexBuffer&) = delete;

    inline VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    [[nodiscard]] inline bool operator==(const VertexBuffer& other) const;

    [[nodiscard]] inline bool operator<(const VertexBuffer& other) const;

    [[nodiscard]] inline size_t handle() const;

    [[nodiscard]] inline bool is_valid() const;

    inline void invalidate();

private:
    Renderer* m_renderer {};
    size_t m_handle {};
};

namespace detail {
struct Image {
    vk::Image vk_handle;
    VmaAllocation vma_allocation {};
    uint32_t width {};
    uint32_t height {};
};

struct DepthImage {
    Image image;
    vk::ImageView vk_image_view;
};

struct RenderImage {
    Image image;
    vk::ImageView vk_image_view;
};

struct TextureImpl {
    Image image;
    vk::ImageView vk_image_view;
    vk::Sampler vk_sampler;
    uint32_t mip_levels {};
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    [[nodiscard]] bool is_complete() const
    {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
};

using DescriptorSetLayoutHandleImpl = uint64_t;

struct Buffer {
    vk::Buffer vk_handle;
    VmaAllocation vma_allocation {};
};

struct VertexBufferImpl {
    Buffer buffer;
    int vertex_count {};
};

struct IndexBufferImpl {
    Buffer buffer;
    size_t index_count {};
};

struct UniformBufferImpl {
    Buffer buffer;
    uint32_t size {};
    std::byte* mapped_ptr {};
};

struct DescriptorSetImpl {
    uint64_t id {};
    vk::DescriptorSet vk_handle;
    vk::DescriptorPool vk_pool;
};

struct FrameInFlight {
    vk::CommandBuffer command_buffer;
    vk::Semaphore image_available_semaphore;
    vk::Semaphore render_finished_semaphore;
    vk::Fence in_flight_fence;
    std::vector<std::optional<UniformBufferImpl>> uniform_buffers {};
    std::vector<std::optional<DescriptorSetImpl>> descriptor_sets {};
    std::queue<uint32_t> funcs;
};

struct CurrentDrawState {
    bool is_drawing {};
    uint32_t image_index {};
    vk::CommandBuffer command_buffer;
    uint64_t current_pipeline {};
    uint32_t frame_index {};
};

struct DeferredFunction {
    std::function<void(uint32_t)> function;
    int counter;
};

struct GraphicsPipelineLayoutImpl {
    vk::PipelineLayout vk_handle;
    std::unordered_map<uint64_t, DescriptorSetLayoutHandleImpl> descriptor_set_layouts;
};

struct GraphicsPipelineImpl {
    size_t layout {};
    vk::Pipeline pipeline;
};

struct FramebufferImpl {
    std::vector<vk::Framebuffer> vk_framebuffers;
    Texture texture;
    std::optional<std::function<void()>> callback;
    Vector2i size;
};
}

}
