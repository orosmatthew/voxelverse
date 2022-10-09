#pragma once

#ifndef NDEBUG
#define MVE_ENABLE_VALIDATION_LAYERS
#endif

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "logger.hpp"

#include <filesystem>

namespace mve {

    enum class ShaderType {
        e_vertex,
        e_fragment,
    };

    class Shader {
    public:
        Shader(const std::filesystem::path &file_path, ShaderType shader_type, bool optimize);

        [[nodiscard]] std::vector<uint32_t> get_spv_code() const;

    private:
        std::vector<uint32_t> m_spv_code;
    };

    class Renderer {
    public:
        Renderer(
            GLFWwindow *window,
            const std::string &app_name,
            int app_version_major,
            int app_version_minor,
            int app_version_patch,
            const Shader &vertex_shader,
            const Shader &fragment_shader,
            int frames_in_flight = 2);

        ~Renderer();

        void draw_frame();

    private:
        struct VkQueueFamilyIndices {
            std::optional<uint32_t> graphics_family;
            std::optional<uint32_t> present_family;

            [[nodiscard]] bool is_complete() const
            {
                return graphics_family.has_value() && present_family.has_value();
            }
        };

        struct VkSwapchainSupportDetails {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> present_modes;
        };

        const int m_frames_in_flight;
        vk::UniqueInstance m_vk_instance;
        vk::UniqueSurfaceKHR m_vk_surface;
        vk::PhysicalDevice m_vk_physical_device;
        vk::UniqueDevice m_vk_device;
        vk::Queue m_vk_graphics_queue;
        vk::Queue m_vk_present_queue;
        vk::SurfaceFormatKHR m_vk_swapchain_image_format;
        vk::Extent2D m_vk_swapchain_extent;
        vk::UniqueSwapchainKHR m_vk_swapchain;
        std::vector<vk::Image> m_vk_swapchain_images;
        std::vector<vk::UniqueImageView> m_vk_swapchain_image_views;
        vk::UniquePipelineLayout m_vk_pipeline_layout;
        vk::UniqueRenderPass m_vk_render_pass;
        vk::UniquePipeline m_vk_graphics_pipeline;
        std::vector<vk::UniqueFramebuffer> m_vk_swapchain_framebuffers;
        vk::UniqueCommandPool m_vk_command_pool;
        std::vector<vk::UniqueCommandBuffer> m_vk_command_buffers;
        std::vector<vk::UniqueSemaphore> m_vk_image_available_semaphores;
        std::vector<vk::UniqueSemaphore> m_vk_render_finished_semaphores;
        std::vector<vk::UniqueFence> m_vk_in_flight_fences;
        uint32_t m_current_frame = 0;

        void recreate_swapchain();

        static bool has_validation_layer_support();

        static vk::UniqueInstance create_vk_instance(
            const std::string &app_name, int app_version_major, int app_version_minor, int app_version_patch);

        static std::vector<const char *> get_vk_validation_layer_exts();

        static std::vector<const char *> get_vk_required_device_exts();

        static vk::PhysicalDevice pick_vk_physical_device(vk::Instance instance, vk::SurfaceKHR surface);

        static bool is_vk_physical_device_suitable(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

        static vk::UniqueDevice create_vk_logical_device(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

        static VkQueueFamilyIndices get_vk_queue_family_indices(
            vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

        static vk::UniqueSurfaceKHR create_vk_surface(vk::Instance instance, GLFWwindow *window);

        static VkSwapchainSupportDetails get_vk_swapchain_support_details(
            vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

        static vk::SurfaceFormatKHR choose_vk_swapchain_surface_format(
            const std::vector<vk::SurfaceFormatKHR> &available_formats);

        static vk::PresentModeKHR choose_vk_swapchain_present_mode(
            const std::vector<vk::PresentModeKHR> &available_present_modes);

        static vk::Extent2D get_vk_swapchain_extent(const vk::SurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);

        static vk::UniqueSwapchainKHR create_vk_swapchain(
            vk::PhysicalDevice physical_device,
            vk::Device device,
            vk::SurfaceKHR surface,
            vk::SurfaceFormatKHR surface_format,
            vk::Extent2D surface_extent);

        static std::vector<vk::Image> get_vk_swapchain_images(vk::Device device, vk::SwapchainKHR swapchain);

        static std::vector<vk::UniqueImageView> create_vk_swapchain_image_views(
            vk::Device device, const std::vector<vk::Image> &swapchain_images, vk::Format image_format);

        static vk::UniquePipeline create_vk_graphics_pipeline(
            vk::Device device,
            const Shader &vertex_shader,
            const Shader &fragment_shader,
            vk::PipelineLayout pipeline_layout,
            vk::RenderPass render_pass);

        static vk::UniquePipelineLayout create_vk_pipeline_layout(vk::Device device);

        static vk::UniqueRenderPass create_vk_render_pass(vk::Device device, vk::Format swapchain_format);

        static std::vector<vk::UniqueFramebuffer> create_vk_framebuffers(
            vk::Device device,
            const std::vector<vk::UniqueImageView> &swapchain_image_views,
            vk::RenderPass render_pass,
            vk::Extent2D swapchain_extent);

        static vk::UniqueCommandPool create_vk_command_pool(
            vk::PhysicalDevice physical_device, vk::SurfaceKHR, vk::Device device);

        static std::vector<vk::UniqueCommandBuffer> create_vk_command_buffers(
            vk::Device device, vk::CommandPool command_pool, int frames_in_flight);

        static void record_vk_command_buffer(
            vk::CommandBuffer command_buffer,
            uint32_t image_index,
            vk::RenderPass render_pass,
            const std::vector<vk::UniqueFramebuffer> &swapchain_framebuffers,
            vk::Extent2D swapchain_extent,
            vk::Pipeline graphics_pipeline);
    };

}