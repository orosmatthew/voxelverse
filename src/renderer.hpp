#pragma once

#include <filesystem>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <strong_type/strong_type.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "uniform_struct_layout.hpp"
#include "vertex_data.hpp"

#ifndef NDEBUG
#define MVE_ENABLE_VALIDATION_LAYERS
#endif

namespace mve {

class Shader;
class Window;

/**
 * @brief Vulkan renderer class
 */
class Renderer {
public:
    /**
     * @brief Handle for GPU resources
     */
    using ResourceHandle = strong::type<uint32_t, struct _resource_handle, strong::regular, strong::hashable>;

    /**
     * @brief Handle for GPU vertex buffer
     */
    using VertexBufferHandle
        = strong::type<ResourceHandle, struct _vertex_data_handle, strong::regular, strong::hashable>;

    /**
     * @brief Handle for GPU index buffer
     */
    using IndexBufferHandle
        = strong::type<ResourceHandle, struct _index_buffer_handle, strong::regular, strong::hashable>;

    /**
     * @brief Handle for uniform buffer
     */
    using UniformBufferHandle
        = strong::type<ResourceHandle, struct _uniform_buffer_handle, strong::regular, strong::hashable>;

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
        int app_version_patch,
        const Shader& vertex_shader,
        const Shader& fragment_shader,
        const VertexLayout& layout,
        int frames_in_flight = 2);

    ~Renderer();

    /**
     * @brief Begin recording commands
     * @param window - Window
     */
    void begin(const Window& window);

    /**
     * @brief Bind and draw vertex buffer
     * @param handle - Vertex buffer handle
     */
    void draw(VertexBufferHandle handle);

    /**
     * @brief Bind vertex buffer
     * @param handle - Vertex buffer handle
     */
    void bind(VertexBufferHandle handle);

    /**
     * @brief Bind and draw index buffer
     * @param handle - Index buffer handle
     */
    void draw(IndexBufferHandle handle);

    /**
     * @brief End recording commands
     * @param window - Window
     */
    void end(const Window& window);

    /**
     * @brief Upload vertex data to GPU vertex buffer
     * @param vertex_data - Vertex data to upload
     * @return - Returns handle GPU vertex buffer
     */
    VertexBufferHandle upload(const VertexData& vertex_data);

    /**
     * @brief Upload index data to GPU index buffer
     * @param index_data - Index data to upload
     * @return - Returns handle to GPU index buffer
     */
    IndexBufferHandle upload(const std::vector<uint32_t>& index_data);

    /**
     * @brief Create a uniform buffer from a given layout
     * @param struct_layout - Uniform struct layout
     * @return - Returns handle to uniform buffer
     */
    UniformBufferHandle create_uniform_buffer(const UniformStructLayout& struct_layout);

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

    void update_uniform(UniformBufferHandle handle, UniformLocation location, glm::mat4 value);

    void bind(UniformBufferHandle handle);

    [[nodiscard]] glm::ivec2 get_extent() const;

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

    struct VertexBuffer {
        Buffer buffer;
        int vertex_count;
    };

    struct IndexBuffer {
        Buffer buffer;
        size_t index_count;
    };

    struct UniformBuffer {
        Buffer buffer;
        std::byte* mapped_ptr;
    };

    struct FrameInFlight {
        vk::CommandBuffer command_buffer;
        vk::Semaphore image_available_semaphore;
        vk::Semaphore render_finished_semaphore;
        vk::Fence in_flight_fence;
        std::unordered_map<UniformBufferHandle, UniformBuffer> uniform_buffers {};
        std::unordered_map<UniformBufferHandle, vk::DescriptorSet> descriptor_sets {};
    };

    struct CurrentDrawState {
        bool is_drawing;
        uint32_t image_index;
        vk::CommandBuffer command_buffer;
        uint32_t frame_index;
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
    vk::PipelineLayout m_vk_pipeline_layout;
    vk::RenderPass m_vk_render_pass;
    vk::Pipeline m_vk_graphics_pipeline;
    std::vector<vk::Framebuffer> m_vk_swapchain_framebuffers;
    vk::CommandPool m_vk_command_pool;
    QueueFamilyIndices m_vk_queue_family_indices;
    vk::DescriptorSetLayout m_vk_descriptor_set_layout;
    vk::DescriptorPool m_vk_descriptor_pool;
    VmaAllocator m_vma_allocator;
    uint32_t m_resource_handle_count;
    CurrentDrawState m_current_draw_state;

    std::vector<FrameInFlight> m_frames_in_flight;

    std::unordered_map<VertexBufferHandle, VertexBuffer> m_vertex_buffers;
    std::unordered_map<VertexBufferHandle, int> m_vertex_buffer_deletion_queue;

    std::unordered_map<IndexBufferHandle, IndexBuffer> m_index_buffers;
    std::unordered_map<IndexBufferHandle, int> m_index_buffer_deletion_queue;

    void cleanup_vk_swapchain();

    void cleanup_vk_debug_messenger();

    void recreate_swapchain(const Window& window);

    static VertexBuffer create_vertex_buffer(
        vk::Device device,
        vk::CommandPool command_pool,
        vk::Queue graphics_queue,
        VmaAllocator allocator,
        const VertexData& vertex_data);

    static IndexBuffer create_index_buffer(
        vk::Device device,
        vk::CommandPool command_pool,
        vk::Queue graphics_queue,
        VmaAllocator allocator,
        const std::vector<uint32_t>& index_data);

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
        const VertexLayout& layout);

    static vk::PipelineLayout create_vk_pipeline_layout(
        vk::Device device, vk::DescriptorSetLayout descriptor_set_layout);

    static vk::RenderPass create_vk_render_pass(vk::Device device, vk::Format swapchain_format);

    static std::vector<vk::Framebuffer> create_vk_framebuffers(
        vk::Device device,
        const std::vector<vk::ImageView>& swapchain_image_views,
        vk::RenderPass render_pass,
        vk::Extent2D swapchain_extent);

    static vk::CommandPool create_vk_command_pool(vk::Device device, QueueFamilyIndices queue_family_indices);

    static std::vector<vk::CommandBuffer> create_vk_command_buffers(
        vk::Device device, vk::CommandPool command_pool, int frames_in_flight);

    static Buffer create_buffer(
        VmaAllocator allocator,
        size_t size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memory_usage,
        VmaAllocationCreateFlags flags = 0);

    static void copy_buffer(
        vk::Device device,
        vk::CommandPool command_pool,
        vk::Queue graphics_queue,
        vk::Buffer src_buffer,
        vk::Buffer dst_buffer,
        vk::DeviceSize size);

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