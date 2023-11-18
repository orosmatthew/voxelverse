#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define VMA_VULKAN_VERSION 1001000
#include "vk_mem_alloc.h"
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include <mve/detail/defs.hpp>
#include <mve/detail/descriptor_set_allocator.hpp>
#include <mve/shader.hpp>
#include <mve/vertex_data.hpp>

namespace mve {

class Window;
class ShaderDescriptorSet;

enum class TextureFormat { r, rg, rgb, rgba };

class Renderer {
public:
    Renderer(
        const Window& window,
        const std::string& app_name,
        int app_version_major,
        int app_version_minor,
        int app_version_patch);

    ~Renderer();

    void begin_frame(const Window& window);

    void begin_render_pass_present() const;

    void begin_render_pass_framebuffer(const Framebuffer& framebuffer) const;

    void draw_vertex_buffer(const VertexBuffer& vertex_buffer);

    void bind_vertex_buffer(const VertexBuffer& vertex_buffer) const;

    void bind_graphics_pipeline(const GraphicsPipeline& graphics_pipeline);

    void draw_index_buffer(const IndexBuffer& index_buffer);

    void end_frame(const Window& window);

    void end_render_pass_present() const;

    void end_render_pass_framebuffer() const;

    void resize(const Window& window);

    VertexBuffer create_vertex_buffer(const VertexData& vertex_data);

    IndexBuffer create_index_buffer(const std::vector<uint32_t>& indices);

    DescriptorSet create_descriptor_set(
        const GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set);

    UniformBuffer create_uniform_buffer(const ShaderDescriptorBinding& descriptor_binding);

    void write_descriptor_binding(
        const DescriptorSet& descriptor_set,
        const ShaderDescriptorBinding& descriptor_binding,
        const UniformBuffer& uniform_buffer);

    void write_descriptor_binding(
        const DescriptorSet& descriptor_set, const ShaderDescriptorBinding& descriptor_binding, const Texture& texture);

    Texture create_texture(const std::filesystem::path& path);

    Texture create_texture(TextureFormat format, uint32_t width, uint32_t height, const std::byte* data);

    GraphicsPipeline create_graphics_pipeline(
        const Shader& vertex_shader,
        const Shader& fragment_shader,
        const VertexLayout& vertex_layout,
        bool depth_test = true);

    void destroy(VertexBuffer& vertex_buffer);

    void destroy(IndexBuffer& index_buffer);

    void destroy(GraphicsPipeline& graphics_pipeline);

    void destroy(DescriptorSet& descriptor_set);

    void destroy(UniformBuffer& uniform_buffer);

    void destroy(Texture& texture);

    void destroy(Framebuffer& framebuffer);

    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, float value, bool persist = true);

    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, Vector2 value, bool persist = true);

    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, Vector3 value, bool persist = true);

    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, Vector4 value, bool persist = true);

    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, const Matrix3& value, bool persist = true);

    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, const Matrix4& value, bool persist = true);

    void bind_descriptor_set(DescriptorSet& descriptor_set) const;

    void bind_descriptor_sets(const DescriptorSet& descriptor_set_a, const DescriptorSet& descriptor_set_b) const;

    [[nodiscard]] Vector2i extent() const;

    Framebuffer create_framebuffer(std::function<void()> callback);

    [[nodiscard]] Vector2i framebuffer_size(const Framebuffer& framebuffer) const;

    const Texture& framebuffer_texture(const Framebuffer& framebuffer);

    [[nodiscard]] std::string gpu_name() const;

    [[nodiscard]] Vector2i texture_size(const Texture& texture) const;

private:
    static constexpr size_t sc_max_uniform_value_size = std::max(
        { sizeof(float), sizeof(Vector2), sizeof(Vector3), sizeof(Vector4), sizeof(Matrix3), sizeof(Matrix4) });
    const int c_frames_in_flight;
    vk::Instance m_vk_instance;
    vk::DispatchLoaderDynamic m_vk_loader;
    vk::DebugUtilsMessengerEXT m_vk_debug_utils_messenger;
    vk::SurfaceKHR m_vk_surface;
    vk::PhysicalDevice m_vk_physical_device;
    vk::Device m_vk_device;
    vk::Queue m_vk_graphics_queue;
    vk::Queue m_vk_present_queue;
    vk::SurfaceFormatKHR m_vk_swapchain_image_format;
    vk::Extent2D m_vk_swapchain_extent;
    vk::SwapchainKHR m_vk_swapchain;
    std::vector<vk::Image> m_vk_swapchain_images;
    std::vector<vk::ImageView> m_vk_swapchain_image_views;
    vk::RenderPass m_vk_render_pass;
    vk::RenderPass m_vk_render_pass_framebuffer;
    std::vector<vk::Framebuffer> m_vk_swapchain_framebuffers;
    vk::CommandPool m_vk_command_pool;
    detail::QueueFamilyIndices m_vk_queue_family_indices;
    VmaAllocator m_vma_allocator {};
    uint64_t m_resource_handle_count;
    detail::CurrentDrawState m_current_draw_state;
    detail::DescriptorSetAllocator m_descriptor_set_allocator {};
    detail::DepthImage m_depth_image;
    vk::SampleCountFlagBits m_msaa_samples;
    detail::RenderImage m_color_image;

    void bind_descriptor_sets(uint32_t num, const std::array<const DescriptorSet*, 4>& descriptor_sets) const;

    std::vector<detail::FrameInFlight> m_frames_in_flight;

    std::vector<std::optional<detail::VertexBufferImpl>> m_vertex_buffers;

    std::vector<std::optional<detail::IndexBufferImpl>> m_index_buffers;

    std::unordered_map<detail::DescriptorSetLayoutHandleImpl, vk::DescriptorSetLayout> m_descriptor_set_layouts;

    std::vector<std::optional<detail::GraphicsPipelineImpl>> m_graphics_pipelines {};

    std::vector<std::optional<detail::GraphicsPipelineLayoutImpl>> m_graphics_pipeline_layouts {};

    std::vector<std::optional<detail::FramebufferImpl>> m_framebuffers {};

    std::unordered_map<uint64_t, detail::TextureImpl> m_textures {};

    uint32_t m_deferred_function_id_count;
    std::map<uint32_t, detail::DeferredFunction> m_deferred_functions;
    std::queue<uint32_t> m_wait_frames_deferred_functions {};
    std::queue<std::function<void(vk::CommandBuffer)>> m_command_buffer_deferred_functions {};

    template <typename T>
    void update_uniform(UniformBuffer& uniform_buffer, const UniformLocation location, T value, const bool persist)
    {
        static_assert(sizeof(T) <= sc_max_uniform_value_size);
        const uint64_t handle = uniform_buffer.handle();
        DeferredUniformUpdateData update_data {
            .counter = persist ? c_frames_in_flight : 1,
            .handle = handle,
            .location = location,
            .data = {},
            .data_size = sizeof(T)
        };
        memcpy(update_data.data.data(), &value, sizeof(T));
        m_deferred_uniform_updates.push_back(update_data);
    }

    struct DeferredUniformUpdateData {
        int counter;
        uint64_t handle;
        UniformLocation location;
        std::array<std::byte, sc_max_uniform_value_size> data;
        size_t data_size;
    };

    enum class DescriptorBindingType { uniform_buffer, texture };

    struct DeferredDescriptorWriteData {
        int counter;
        DescriptorBindingType data_type;
        uint64_t data_handle;
        uint64_t descriptor_handle;
        uint32_t binding;
    };

    std::vector<DeferredUniformUpdateData> m_deferred_uniform_updates {};
    std::vector<DeferredDescriptorWriteData> m_deferred_descriptor_writes {};

    void cleanup_vk_swapchain() const;

    void cleanup_vk_debug_messenger() const;

    void recreate_swapchain(const Window& window);

    void recreate_framebuffers();

    Texture create_texture(
        const detail::Image& image, vk::ImageView image_view, vk::Sampler sampler, uint32_t mip_levels);

    detail::FramebufferImpl create_framebuffer_impl(
        const vk::DispatchLoaderDynamic& loader, std::optional<std::function<void()>> callback);

    void wait_ready() const;

    detail::DescriptorSetLayoutHandleImpl create_descriptor_set_layout(
        const vk::DispatchLoaderDynamic& loader,
        uint32_t set,
        const Shader& vertex_shader,
        const Shader& fragment_shader);

    size_t create_graphics_pipeline_layout(
        const vk::DispatchLoaderDynamic& loader, const Shader& vertex_shader, const Shader& fragment_shader);

    void update_uniform(
        size_t handle, UniformLocation location, const void* data_ptr, size_t size, uint32_t frame_index) const;

    void defer_to_all_frames(std::function<void(uint32_t)> func);

    void defer_to_next_frame(std::function<void(uint32_t)> func);

    void defer_after_all_frames(std::function<void(uint32_t)> func);

    void defer_to_command_buffer_front(const std::function<void(vk::CommandBuffer)>& func);

    [[nodiscard]] vk::PipelineLayout create_vk_pipeline_layout(
        const vk::DispatchLoaderDynamic& loader,
        const std::vector<detail::DescriptorSetLayoutHandleImpl>& layouts) const;

    static std::vector<vk::DescriptorSet> create_vk_descriptor_sets(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        vk::DescriptorSetLayout layout,
        vk::DescriptorPool pool,
        int count);
};
}

#include "detail/types.hpp"