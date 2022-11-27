#pragma once

#include <filesystem>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <strong_type/strong_type.hpp>
#include <glm/glm.hpp>
#include <vk_mem_alloc.h>

#include "vertex_data.hpp"

#ifndef NDEBUG
#define MVE_ENABLE_VALIDATION_LAYERS
#endif

namespace mve {

    class Shader;
    class Window;

    class Renderer {
    public:
        using ResourceHandle = strong::type<uint32_t, struct _resource_handle, strong::regular, strong::hashable>;
        using VertexBufferHandle
            = strong::type<ResourceHandle, struct _vertex_data_handle, strong::regular, strong::hashable>;
        using IndexBufferHandle
            = strong::type<ResourceHandle, struct _index_buffer_handle, strong::regular, strong::hashable>;

        Renderer(
            const Window &window,
            const std::string &app_name,
            int app_version_major,
            int app_version_minor,
            int app_version_patch,
            const Shader &vertex_shader,
            const Shader &fragment_shader,
            const VertexLayout &layout,
            int frames_in_flight = 2);

        ~Renderer();

        void recreate_swapchain(const Window &window);

        void begin(const Window &window);

        void draw(VertexBufferHandle handle);

        void end(const Window &window);

        VertexBufferHandle upload_vertex_data(const VertexData &vertex_data);

        IndexBufferHandle upload_index_data(const std::vector<uint32_t> &index_data);

        void queue_destroy(VertexBufferHandle handle);

        void queue_destroy(IndexBufferHandle handle);

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

        struct FrameInFlight {
            vk::CommandBuffer command_buffer;
            vk::Semaphore image_available_semaphore;
            vk::Semaphore render_finished_semaphore;
            vk::Fence in_flight_fence;
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
        uint32_t m_current_frame = 0;
        VmaAllocator m_vma_allocator;
        uint32_t m_resource_handle_count;

        uint32_t m_current_image_index;
        vk::CommandBuffer m_current_command_buffer;

        std::vector<FrameInFlight> m_frames_in_flight;

        std::unordered_map<VertexBufferHandle, VertexBuffer> m_vertex_buffers;
        std::unordered_map<VertexBufferHandle, int> m_vertex_buffer_deletion_queue;

        std::unordered_map<IndexBufferHandle, IndexBuffer> m_index_buffers;
        std::unordered_map<IndexBufferHandle, int> m_index_buffer_deletion_queue;

        void cleanup_vk_swapchain();

        void cleanup_vk_debug_messenger();

        VertexBuffer create_vertex_buffer(const VertexData &vertex_data);

        IndexBuffer create_index_buffer(const std::vector<uint32_t> &index_data);

        static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
            VkDebugUtilsMessageTypeFlagsEXT msg_type,
            const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
            void *user_data);

        static bool has_validation_layer_support();

        static vk::Instance create_vk_instance(
            const std::string &app_name, int app_version_major, int app_version_minor, int app_version_patch);

        static vk::DebugUtilsMessengerEXT create_vk_debug_messenger(vk::Instance instance);

        static std::vector<const char *> get_vk_validation_layer_exts();

        static std::vector<const char *> get_vk_device_required_exts();

        static std::vector<const char *> get_vk_instance_required_exts();

        static vk::PhysicalDevice pick_vk_physical_device(vk::Instance instance, vk::SurfaceKHR surface);

        static bool is_vk_physical_device_suitable(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

        static vk::Device create_vk_logical_device(
            vk::PhysicalDevice physical_device, QueueFamilyIndices queue_family_indices);

        static QueueFamilyIndices get_vk_queue_family_indices(
            vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

        static vk::SurfaceKHR create_vk_surface(vk::Instance instance, GLFWwindow *window);

        static SwapchainSupportDetails get_vk_swapchain_support_details(
            vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

        static vk::SurfaceFormatKHR choose_vk_swapchain_surface_format(
            const std::vector<vk::SurfaceFormatKHR> &available_formats);

        static vk::PresentModeKHR choose_vk_swapchain_present_mode(
            const std::vector<vk::PresentModeKHR> &available_present_modes);

        static vk::Extent2D get_vk_swapchain_extent(const vk::SurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);

        static vk::SwapchainKHR create_vk_swapchain(
            vk::PhysicalDevice physical_device,
            vk::Device device,
            vk::SurfaceKHR surface,
            vk::SurfaceFormatKHR surface_format,
            vk::Extent2D surface_extent,
            QueueFamilyIndices queue_family_indices);

        static std::vector<vk::Image> get_vk_swapchain_images(vk::Device device, vk::SwapchainKHR swapchain);

        static std::vector<vk::ImageView> create_vk_swapchain_image_views(
            vk::Device device, const std::vector<vk::Image> &swapchain_images, vk::Format image_format);

        static vk::Pipeline create_vk_graphics_pipeline(
            vk::Device device,
            const Shader &vertex_shader,
            const Shader &fragment_shader,
            vk::PipelineLayout pipeline_layout,
            vk::RenderPass render_pass,
            const VertexLayout &layout);

        static vk::PipelineLayout create_vk_pipeline_layout(vk::Device device);

        static vk::RenderPass create_vk_render_pass(vk::Device device, vk::Format swapchain_format);

        static std::vector<vk::Framebuffer> create_vk_framebuffers(
            vk::Device device,
            const std::vector<vk::ImageView> &swapchain_image_views,
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
            VmaAllocationCreateFlags flags);

        static void copy_buffer(
            vk::Device device,
            vk::CommandPool command_pool,
            vk::Queue graphics_queue,
            vk::Buffer src_buffer,
            vk::Buffer dst_buffer,
            vk::DeviceSize size);

        static vk::VertexInputBindingDescription create_vk_binding_description(const VertexLayout &layout);

        static std::vector<vk::VertexInputAttributeDescription> create_vk_attribute_descriptions(
            const VertexLayout &layout);

        static std::vector<FrameInFlight> create_frames_in_flight(
            vk::Device device, vk::CommandPool command_pool, int frame_count);
    };
}