#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

#include "descriptor_set.hpp"
#include "graphics_pipeline.hpp"
#include "index_buffer.hpp"
#include "texture.hpp"
#include "uniform_buffer.hpp"
#include "vertex_buffer.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <strong_type/strong_type.hpp>
#include <vk_mem_alloc.h>
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "shader.hpp"
#include "vertex_data.hpp"

#ifndef NDEBUG
#define MVE_ENABLE_VALIDATION_LAYERS
#endif

namespace mve {

class Window;
class ShaderDescriptorSet;

/**
 * @brief Types of bindings for descriptor set
 */
enum DescriptorType {
    e_uniform_buffer,
    e_combined_image_sampler,
};

/**
 * @brief Handle for GPU resources
 */
using ResourceHandle = strong::type<uint32_t, struct _resource_handle, strong::regular, strong::hashable>;

/**
 * @brief Handle for a descriptor set layout
 */
using DescriptorSetLayoutHandle
    = strong::type<ResourceHandle, struct _descriptor_set_layout_handle, strong::regular, strong::hashable>;

using GraphicsPipelineLayoutHandle
    = strong::type<ResourceHandle, struct _graphics_pipeline_layout_handle, strong::regular, strong::hashable>;

/**
 * @brief Vulkan renderer class
 */
class Renderer {
public:
    /**
     * @brief Construct Vulkan renderer
     * @param window - Window
     * @param app_name - Name of application
     * @param app_version_major - App version major
     * @param app_version_minor - App version minor
     * @param app_version_patch - App version patch
     * @param vertex_shader - Vertex shader
     * @param fragment_shader - Fragment shader
     * @param layout - Vertex layout
     * @param frames_in_flight - Number of frames in flight
     */
    Renderer(
        const Window& window,
        const std::string& app_name,
        int app_version_major,
        int app_version_minor,
        int app_version_patch);

    ~Renderer();

    /**
     * @brief Begin recording commands
     */
    void begin(const Window& window);

    /**
     * @brief Bind and draw vertex buffer
     * @param handle - Vertex buffer handle
     */
    void draw_vertex_buffer(VertexBufferHandle handle);

    /**
     * @brief Bind vertex buffer
     * @param handle - Vertex buffer handle
     */
    void bind_vertex_buffer(VertexBufferHandle handle);

    void bind_vertex_buffer(const VertexBuffer& vertex_buffer);

    void bind_graphics_pipeline(GraphicsPipelineHandle handle);

    void bind_graphics_pipeline(GraphicsPipeline& graphics_pipeline);

    /**
     * @brief Bind and draw index buffer
     * @param handle - Index buffer handle
     */
    void draw_index_buffer(IndexBufferHandle handle);

    void draw_index_buffer(const IndexBuffer& index_buffer);

    /**
     * @brief End recording commands
     * @param window - Window
     */
    void end(const Window& window);

    void resize(const Window& window);

    /**
     * @brief Upload vertex data to GPU vertex buffer
     * @param vertex_data - Vertex data
     * @return - Returns handle GPU vertex buffer
     */
    VertexBufferHandle create_vertex_buffer_handle(const VertexData& vertex_data);

    VertexBuffer create_vertex_buffer(const VertexData& vertex_data);

    /**
     * @brief Upload index data to GPU index buffer
     * @param index_data - Index data
     * @return - Returns handle to GPU index buffer
     */
    IndexBufferHandle create_index_buffer_handle(const std::vector<uint32_t>& index_data);

    IndexBuffer create_index_buffer(const std::vector<uint32_t>& indices);

    /**
     * @brief Upload descriptor set layout to GPU
     * @param layout - Layout to create
     * @return - Returns handle to GPU descriptor set layout
     */
    DescriptorSetLayoutHandle create_descriptor_set_layout(const std::vector<DescriptorType>& layout);

    DescriptorSetLayoutHandle create_descriptor_set_layout(
        uint32_t set, const Shader& vertex_shader, const Shader& fragment_shader);

    DescriptorSetHandle create_descriptor_set_handle(GraphicsPipelineHandle pipeline, uint32_t set);

    DescriptorSetHandle create_descriptor_set_handle(GraphicsPipeline& pipeline, uint32_t set);

    DescriptorSet create_descriptor_set(GraphicsPipelineHandle pipeline, uint32_t set);

    DescriptorSet create_descriptor_set(GraphicsPipeline& pipeline, uint32_t set);

    /**
     * @brief Create a uniform buffer from a given layout
     * @param struct_layout - Uniform struct layout
     * @return - Returns handle to uniform buffer
     */
    UniformBufferHandle create_uniform_buffer_handle(const ShaderDescriptorBinding& binding);

    UniformBuffer create_uniform_buffer(const ShaderDescriptorBinding& binding);

    void write_descriptor_binding_uniform(
        DescriptorSetHandle descriptor_set, const ShaderDescriptorBinding& binding, UniformBufferHandle uniform_buffer);

    void write_descriptor_binding_uniform(
        DescriptorSet& descriptor_set, const ShaderDescriptorBinding& binding, UniformBuffer& uniform_buffer);

    void write_descriptor_binding_texture(
        DescriptorSetHandle descriptor_set, const ShaderDescriptorBinding& binding, TextureHandle texture);

    void write_descriptor_binding_texture(
        DescriptorSet& descriptor_set, const ShaderDescriptorBinding& binding, Texture& texture);

    // TODO: make TextureHandle to Texture class
    void write_descriptor_binding_texture(
        DescriptorSet& descriptor_set, const ShaderDescriptorBinding& binding, TextureHandle texture);

    TextureHandle create_texture_handle(const std::filesystem::path& path);

    Texture create_texture(const std::filesystem::path& path);

    GraphicsPipelineLayoutHandle create_graphics_pipeline_layout(
        const mve::Shader& vertex_shader, const mve::Shader& fragment_shader);

    GraphicsPipelineHandle create_graphics_pipeline_handle(
        const Shader& vertex_shader, const Shader& fragment_shader, const VertexLayout& vertex_layout);

    GraphicsPipeline create_graphics_pipeline(
        const Shader& vertex_shader, const Shader& fragment_shader, const VertexLayout& vertex_layout);

    /**
     * @brief Determines if vertex buffer handle is valid
     * @param handle - Vertex buffer handle
     * @return - Returns true if valid
     */
    bool is_valid(VertexBufferHandle handle);

    /**
     * @brief Determines if index buffer handle is valid
     * @param handle - Index buffer handle
     * @return - Returns true if valid
     */
    bool is_valid(IndexBufferHandle handle);

    /**
     * @brief Destroy GPU vertex buffer
     * @param handle - Vertex buffer handle
     */
    void queue_destroy(VertexBufferHandle handle);

    /**
     * @brief Destroy GPU index buffer
     * @param handle - Index buffer handle
     */
    void queue_destroy(IndexBufferHandle handle);

    void queue_destroy(GraphicsPipelineHandle handle);

    void queue_destroy(DescriptorSetHandle handle);

    void queue_destroy(UniformBufferHandle handle);

    void queue_destroy(TextureHandle handle);

    void update_uniform(UniformBufferHandle handle, UniformLocation location, float value, bool persist = true);
    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, float value, bool persist = true);

    void update_uniform(UniformBufferHandle handle, UniformLocation location, glm::vec2 value, bool persist = true);
    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, glm::vec2 value, bool persist = true);

    void update_uniform(UniformBufferHandle handle, UniformLocation location, glm::vec3 value, bool persist = true);
    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, glm::vec3 value, bool persist = true);

    void update_uniform(UniformBufferHandle handle, UniformLocation location, glm::vec4 value, bool persist = true);
    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, glm::vec4 value, bool persist = true);

    void update_uniform(UniformBufferHandle handle, UniformLocation location, glm::mat2 value, bool persist = true);
    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, glm::mat2 value, bool persist = true);

    void update_uniform(UniformBufferHandle handle, UniformLocation location, glm::mat3 value, bool persist = true);
    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, glm::mat3 value, bool persist = true);

    void update_uniform(UniformBufferHandle handle, UniformLocation location, glm::mat4 value, bool persist = true);
    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, glm::mat4 value, bool persist = true);

    void bind_descriptor_set(DescriptorSetHandle handle);

    void bind_descriptor_set(DescriptorSet& descriptor_set);

    [[nodiscard]] glm::ivec2 extent() const;

    void wait_ready();

private:
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

    struct Buffer {
        vk::Buffer vk_handle;
        VmaAllocation vma_allocation;
    };

    struct VertexBufferImpl {
        Buffer buffer;
        int vertex_count;
    };

    struct IndexBufferImpl {
        Buffer buffer;
        size_t index_count;
    };

    struct UniformBufferImpl {
        Buffer buffer;
        uint32_t size;
        std::byte* mapped_ptr;
    };

    struct Image {
        vk::Image vk_handle;
        VmaAllocation vma_allocation;
    };

    struct RenderImage {
        Image image;
        vk::ImageView vk_image_view;
    };

    struct DepthImage {
        Image image;
        vk::ImageView vk_image_view;
    };

    struct TextureImpl {
        Image image;
        vk::ImageView vk_image_view;
        vk::Sampler vk_sampler;
        uint32_t mip_levels;
    };

    struct DescriptorSetImpl {
        uint64_t id;
        vk::DescriptorSet vk_handle;
        vk::DescriptorPool vk_pool;
    };

    struct FrameInFlight {
        vk::CommandBuffer command_buffer;
        vk::Semaphore image_available_semaphore;
        vk::Semaphore render_finished_semaphore;
        vk::Fence in_flight_fence;
        std::unordered_map<UniformBufferHandle, UniformBufferImpl> uniform_buffers {};
        std::unordered_map<DescriptorSetHandle, DescriptorSetImpl> descriptor_sets {};
        std::queue<uint32_t> funcs;
    };

    struct CurrentDrawState {
        bool is_drawing;
        uint32_t image_index;
        vk::CommandBuffer command_buffer;
        GraphicsPipelineHandle current_pipeline;
        uint32_t frame_index;
    };

    struct DeferredFunction {
        std::function<void(uint32_t)> function;
        int counter;
    };

    struct GraphicsPipelineLayoutImpl {
        vk::PipelineLayout vk_handle;
        std::unordered_map<uint32_t, DescriptorSetLayoutHandle> descriptor_set_layouts;
    };

    struct GraphicsPipelineImpl {
        GraphicsPipelineLayoutHandle layout;
        vk::Pipeline pipeline;
    };

    class DescriptorSetAllocator {
    public:
        DescriptorSetAllocator();

        void cleanup(vk::Device device);

        void free(vk::Device device, DescriptorSetImpl descriptor_set);

        DescriptorSetImpl create(vk::Device device, vk::DescriptorSetLayout layout);

    private:
        std::vector<std::pair<vk::DescriptorType, float>> m_sizes;
        uint32_t m_max_sets_per_pool;

        uint64_t m_id_count;
        std::vector<vk::DescriptorPool> m_descriptor_pools {};
        std::unordered_map<uint64_t, DescriptorSetImpl> m_descriptor_sets {};
        size_t m_current_pool_index;

        static std::optional<vk::DescriptorSet> try_create(
            vk::DescriptorPool pool, vk::Device device, vk::DescriptorSetLayout layout);

        vk::DescriptorPool create_pool(
            vk::Device device, vk::DescriptorPoolCreateFlags flags = vk::DescriptorPoolCreateFlags());
    };

    const int c_frames_in_flight;
    vk::Instance m_vk_instance;
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
    std::vector<vk::Framebuffer> m_vk_swapchain_framebuffers;
    vk::CommandPool m_vk_command_pool;
    QueueFamilyIndices m_vk_queue_family_indices;
    VmaAllocator m_vma_allocator;
    uint32_t m_resource_handle_count;
    CurrentDrawState m_current_draw_state;
    DescriptorSetAllocator m_descriptor_set_allocator {};
    DepthImage m_depth_image;
    vk::SampleCountFlagBits m_msaa_samples;
    RenderImage m_color_image;

    std::vector<FrameInFlight> m_frames_in_flight;

    std::unordered_map<VertexBufferHandle, VertexBufferImpl> m_vertex_buffers;

    std::unordered_map<IndexBufferHandle, IndexBufferImpl> m_index_buffers;

    std::unordered_map<DescriptorSetLayoutHandle, vk::DescriptorSetLayout> m_descriptor_set_layouts;

    std::unordered_map<GraphicsPipelineHandle, GraphicsPipelineImpl> m_graphics_pipelines {};

    std::unordered_map<GraphicsPipelineLayoutHandle, GraphicsPipelineLayoutImpl> m_graphics_pipeline_layouts {};

    std::unordered_map<TextureHandle, TextureImpl> m_textures {};

    uint32_t m_deferred_function_id_count;
    std::map<uint32_t, DeferredFunction> m_deferred_functions;
    std::queue<uint32_t> m_wait_frames_deferred_functions {};
    std::queue<std::function<void(vk::CommandBuffer)>> m_command_buffer_deferred_functions {};

    void cleanup_vk_swapchain();

    void cleanup_vk_debug_messenger();

    void recreate_swapchain(const Window& window);

    static vk::SampleCountFlagBits get_max_sample_count(vk::PhysicalDevice physical_device);

    static DepthImage create_depth_image(
        vk::PhysicalDevice physical_device,
        vk::Device device,
        vk::CommandPool pool,
        vk::Queue queue,
        VmaAllocator allocator,
        vk::Extent2D extent,
        vk::SampleCountFlagBits samples);

    static vk::Format find_supported_format(
        vk::PhysicalDevice physical_device,
        const std::vector<vk::Format>& formats,
        vk::ImageTiling tiling,
        vk::FormatFeatureFlags features);

    static vk::Format find_depth_format(vk::PhysicalDevice physical_device);

    void update_uniform(
        UniformBufferHandle handle, UniformLocation location, void* data_ptr, size_t size, uint32_t frame_index);

    static bool has_stencil_component(vk::Format format);

    static Image create_image(
        VmaAllocator allocator,
        uint32_t width,
        uint32_t height,
        uint32_t mip_levels,
        vk::SampleCountFlagBits samples,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage);

    void defer_to_all_frames(std::function<void(uint32_t)> func);

    void defer_to_next_frame(std::function<void(uint32_t)> func);

    void defer_after_all_frames(std::function<void(uint32_t)> func);

    void defer_to_command_buffer_front(std::function<void(vk::CommandBuffer)> func);

    static RenderImage create_color_image(
        vk::Device device,
        VmaAllocator allocator,
        vk::Extent2D swapchain_extent,
        vk::Format swapchain_format,
        vk::SampleCountFlagBits samples);

    static void cmd_generate_mipmaps(
        vk::PhysicalDevice physical_device,
        vk::CommandBuffer command_buffer,
        vk::Image image,
        vk::Format format,
        uint32_t width,
        uint32_t height,
        uint32_t mip_levels);

    static vk::CommandBuffer begin_single_submit(vk::Device device, vk::CommandPool pool);

    static void end_single_submit(
        vk::Device device, vk::CommandPool pool, vk::CommandBuffer command_buffer, vk::Queue queue);

    static void cmd_transition_image_layout(
        vk::CommandBuffer command_buffer,
        vk::Image image,
        vk::Format format,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        uint32_t mip_levels);

    static void cmd_copy_buffer_to_image(
        vk::CommandBuffer command_buffer, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

    static vk::Sampler create_texture_sampler(
        vk::PhysicalDevice physical_device, vk::Device device, uint32_t mip_levels);

    static vk::ImageView create_image_view(
        vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels);

    static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
        VkDebugUtilsMessageTypeFlagsEXT msg_type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data);

    static bool has_validation_layer_support();

    static vk::Instance create_vk_instance(
        const std::string& app_name, int app_version_major, int app_version_minor, int app_version_patch);

    static vk::DebugUtilsMessengerEXT create_vk_debug_messenger(vk::Instance instance);

    static std::vector<const char*> get_vk_validation_layer_exts();

    static std::vector<const char*> get_vk_device_required_exts();

    static std::vector<const char*> get_vk_instance_required_exts();

    static vk::PhysicalDevice pick_vk_physical_device(vk::Instance instance, vk::SurfaceKHR surface);

    static bool is_vk_physical_device_suitable(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

    static vk::Device create_vk_logical_device(
        vk::PhysicalDevice physical_device, QueueFamilyIndices queue_family_indices);

    static QueueFamilyIndices get_vk_queue_family_indices(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

    static vk::SurfaceKHR create_vk_surface(vk::Instance instance, GLFWwindow* window);

    static SwapchainSupportDetails get_vk_swapchain_support_details(
        vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

    static vk::SurfaceFormatKHR choose_vk_swapchain_surface_format(
        const std::vector<vk::SurfaceFormatKHR>& available_formats);

    static vk::PresentModeKHR choose_vk_swapchain_present_mode(
        const std::vector<vk::PresentModeKHR>& available_present_modes);

    static vk::Extent2D get_vk_swapchain_extent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    static vk::SwapchainKHR create_vk_swapchain(
        vk::PhysicalDevice physical_device,
        vk::Device device,
        vk::SurfaceKHR surface,
        vk::SurfaceFormatKHR surface_format,
        vk::Extent2D surface_extent,
        QueueFamilyIndices queue_family_indices);

    static std::vector<vk::Image> get_vk_swapchain_images(vk::Device device, vk::SwapchainKHR swapchain);

    static std::vector<vk::ImageView> create_vk_swapchain_image_views(
        vk::Device device, const std::vector<vk::Image>& swapchain_images, vk::Format image_format);

    static vk::Pipeline create_vk_graphics_pipeline(
        vk::Device device,
        const Shader& vertex_shader,
        const Shader& fragment_shader,
        vk::PipelineLayout pipeline_layout,
        vk::RenderPass render_pass,
        const VertexLayout& vertex_layout,
        vk::SampleCountFlagBits samples);

    vk::PipelineLayout create_vk_pipeline_layout(const std::vector<DescriptorSetLayoutHandle>& layouts);

    static vk::RenderPass create_vk_render_pass(
        vk::Device device, vk::Format swapchain_format, vk::Format depth_format, vk::SampleCountFlagBits samples);

    static std::vector<vk::Framebuffer> create_vk_framebuffers(
        vk::Device device,
        const std::vector<vk::ImageView>& swapchain_image_views,
        vk::RenderPass render_pass,
        vk::Extent2D swapchain_extent,
        vk::ImageView color_image_view,
        vk::ImageView depth_image_view);

    static vk::CommandPool create_vk_command_pool(vk::Device device, QueueFamilyIndices queue_family_indices);

    static std::vector<vk::CommandBuffer> create_vk_command_buffers(
        vk::Device device, vk::CommandPool command_pool, int frames_in_flight);

    static Buffer create_buffer(
        VmaAllocator allocator,
        size_t size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memory_usage,
        VmaAllocationCreateFlags flags = 0);

    static void cmd_copy_buffer(
        vk::CommandBuffer command_buffer, vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size);

    static vk::VertexInputBindingDescription create_vk_binding_description(const VertexLayout& layout);

    static std::vector<vk::VertexInputAttributeDescription> create_vk_attribute_descriptions(
        const VertexLayout& layout);

    static std::vector<FrameInFlight> create_frames_in_flight(
        vk::Device device, vk::CommandPool command_pool, int frame_count);

    static vk::DescriptorSetLayout create_vk_descriptor_set_layout(vk::Device device);

    static vk::DescriptorPool create_vk_descriptor_pool(vk::Device, int frames_in_flight);

    static std::vector<vk::DescriptorSet> create_vk_descriptor_sets(
        vk::Device device, vk::DescriptorSetLayout layout, vk::DescriptorPool pool, int count);
};
}