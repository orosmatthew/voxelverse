#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

#include "detail/defs.hpp"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vk_mem_alloc.h"
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "shader.hpp"
#include "vertex_data.hpp"

#ifndef NDEBUG
#define MVE_ENABLE_VALIDATION
#endif

// #define MVE_ENABLE_VALIDATION

namespace mve {

// Forward declarations
class Window;
class ShaderDescriptorSet;

enum class TextureFormat { r, rg, rgb, rgba };

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
    void begin_frame(const Window& window);

    void begin_render_pass_present();

    void begin_render_pass_framebuffer(Framebuffer& framebuffer);

    /**
     * @brief Bind and draw vertex buffer
     * @param vertex_buffer
     */
    void draw_vertex_buffer(const VertexBuffer& vertex_buffer);

    /**
     * @brief Bind vertex buffer
     * @param vertex_buffer
     */
    void bind_vertex_buffer(const VertexBuffer& vertex_buffer);

    /**
     * @brief Bind graphics pipeline
     * @param graphics_pipeline
     */
    void bind_graphics_pipeline(const GraphicsPipeline& graphics_pipeline);

    /**
     * @brief Bind and draw index buffer
     * @param index_buffer
     */
    void draw_index_buffer(const IndexBuffer& index_buffer);

    /**
     * @brief End recording commands
     * @param window
     */
    void end_frame(const Window& window);

    void end_render_pass_present();

    void end_render_pass_framebuffer(const Framebuffer& framebuffer);

    /**
     * @brief Resize the renderer. Should be called on window resize
     * @param window
     */
    void resize(const Window& window);

    /**
     * @brief Upload vertex data to GPU vertex buffer
     * @param vertex_data
     * @return Returns vertex buffer object
     */
    VertexBuffer create_vertex_buffer(const VertexData& vertex_data);

    /**
     * @brief Upload index data to GPU index buffer
     * @param indices
     * @return Returns index buffer object
     */
    IndexBuffer create_index_buffer(const std::vector<uint32_t>& indices);

    /**
     * @brief Create descriptor set from shaders used in graphics pipeline
     * @param graphics_pipeline
     * @param descriptor_set
     * @return Returns descriptor set object
     */
    DescriptorSet create_descriptor_set(
        const GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set);

    /**
     * @brief Create a uniform buffer from a shader binding
     * @param descriptor_binding
     * @return Returns a uniform buffer object
     */
    UniformBuffer create_uniform_buffer(const ShaderDescriptorBinding& descriptor_binding);

    /**
     * @brief Write a uniform buffer to a descriptor set binding
     * @param descriptor_set
     * @param descriptor_binding
     * @param uniform_buffer
     */
    void write_descriptor_binding(
        const DescriptorSet& descriptor_set,
        const ShaderDescriptorBinding& descriptor_binding,
        const UniformBuffer& uniform_buffer);

    /**
     * @brief Write a texture to a descriptor set binding
     * @param descriptor_set
     * @param descriptor_binding
     * @param texture
     */
    void write_descriptor_binding(
        const DescriptorSet& descriptor_set, const ShaderDescriptorBinding& descriptor_binding, const Texture& texture);

    /**
     * @brief Create texture from image file-path
     * @param path - File-path to image
     * @return Returns a texture object
     */
    Texture create_texture(const std::filesystem::path& path);

    Texture create_texture(TextureFormat format, uint32_t width, uint32_t height, const std::byte* data);

    /**
     * @brief Create graphics pipeline from shaders and vertex layout
     * @param vertex_shader
     * @param fragment_shader
     * @param vertex_layout
     * @return Returns graphics pipeline object
     */
    GraphicsPipeline create_graphics_pipeline(
        const Shader& vertex_shader,
        const Shader& fragment_shader,
        const VertexLayout& vertex_layout,
        bool depth_test = true);

    /**
     * @brief Destroy vertex buffer manually
     * @param vertex_buffer
     */
    void destroy(VertexBuffer& vertex_buffer);

    /**
     * @brief Destroy index buffer manually
     * @param index_buffer
     */
    void destroy(IndexBuffer& index_buffer);

    /**
     * @brief Destroy graphics pipeline manually
     * @param graphics_pipeline
     */
    void destroy(GraphicsPipeline& graphics_pipeline);

    /**
     * @brief Destroy descriptor set manually
     * @param descriptor_set
     */
    void destroy(DescriptorSet& descriptor_set);

    /**
     * @brief Destroy uniform buffer manually
     * @param uniform_buffer
     */
    void destroy(UniformBuffer& uniform_buffer);

    /**
     * @brief Destroy texture manually
     * @param texture
     */
    void destroy(Texture& texture);

    void destroy(Framebuffer& framebuffer);

    /**
     * @brief Update uniform buffer float
     * @param uniform_buffer
     * @param location - Location is represents a byte offset in the uniform buffer of the variable being set
     * @param value - value to set
     * @param persist - Only set to false if data will update every frame. If true, it will set for all frames
     */
    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, float value, bool persist = true);

    /**
     * @brief Update uniform buffer vec2
     * @param uniform_buffer
     * @param location - Location is represents a byte offset in the uniform buffer of the variable being set
     * @param value - value to set
     * @param persist - Only set to false if data will update every frame. If true, it will set for all frames
     */
    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, mve::Vector2 value, bool persist = true);

    /**
     * @brief Update uniform buffer vec3
     * @param uniform_buffer
     * @param location - Location is represents a byte offset in the uniform buffer of the variable being set
     * @param value - value to set
     * @param persist - Only set to false if data will update every frame. If true, it will set for all frames
     */
    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, mve::Vector3 value, bool persist = true);

    /**
     * @brief Update uniform buffer vec4
     * @param uniform_buffer
     * @param location - Location is represents a byte offset in the uniform buffer of the variable being set
     * @param value - value to set
     * @param persist - Only set to false if data will update every frame. If true, it will set for all frames
     */
    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, mve::Vector4 value, bool persist = true);

    /**
     * @brief Update uniform buffer mat2
     * @param uniform_buffer
     * @param location - Location is represents a byte offset in the uniform buffer of the variable being set
     * @param value - value to set
     * @param persist - Only set to false if data will update every frame. If true, it will set for all frames
     */
    //    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, mve::Matrix value, bool persist =
    //    true);

    /**
     * @brief Update uniform buffer mat3
     * @param uniform_buffer
     * @param location - Location is represents a byte offset in the uniform buffer of the variable being set
     * @param value - value to set
     * @param persist - Only set to false if data will update every frame. If true, it will set for all frames
     */
    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, mve::Matrix3 value, bool persist = true);

    /**
     * @brief Update uniform buffer mat4
     * @param uniform_buffer
     * @param location - Location is represents a byte offset in the uniform buffer of the variable being set
     * @param value - value to set
     * @param persist - Only set to false if data will update every frame. If true, it will set for all frames
     */
    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, mve::Matrix4 value, bool persist = true);

    /**
     * @brief Bind descriptor set
     * @param descriptor_set
     */
    void bind_descriptor_set(DescriptorSet& descriptor_set);

    void bind_descriptor_sets(const DescriptorSet& descriptor_set_a, const DescriptorSet& descriptor_set_b);
    /**
     * @brief Get extent of renderer
     * @return Returns ivec2 of the extent of the renderer
     */
    [[nodiscard]] mve::Vector2i extent() const;

    Framebuffer create_framebuffer(std::function<void(void)> callback);

    Vector2i framebuffer_size(const Framebuffer& framebuffer);

    const Texture& framebuffer_texture(const Framebuffer& framebuffer);

    [[nodiscard]] std::string gpu_name() const;

    [[nodiscard]] mve::Vector2i texture_size(const mve::Texture& texture) const;

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

    using DescriptorSetLayoutHandleImpl = uint64_t;

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
        uint32_t width;
        uint32_t height;
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
        std::vector<std::optional<UniformBufferImpl>> uniform_buffers {};
        std::vector<std::optional<DescriptorSetImpl>> descriptor_sets {};
        std::queue<uint32_t> funcs;
    };

    struct CurrentDrawState {
        bool is_drawing;
        uint32_t image_index;
        vk::CommandBuffer command_buffer;
        uint64_t current_pipeline;
        uint32_t frame_index;
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
        size_t layout;
        vk::Pipeline pipeline;
    };

    struct FramebufferImpl {
        std::vector<vk::Framebuffer> vk_framebuffers;
        Texture texture;
        std::optional<std::function<void(void)>> callback;
        Vector2i size;
    };

    class DescriptorSetAllocator {
    public:
        DescriptorSetAllocator();

        void cleanup(const vk::DispatchLoaderDynamic& loader, vk::Device device);

        void free(const vk::DispatchLoaderDynamic& loader, vk::Device device, DescriptorSetImpl descriptor_set);

        DescriptorSetImpl create(
            const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::DescriptorSetLayout layout);

    private:
        std::vector<std::pair<vk::DescriptorType, float>> m_sizes;
        uint32_t m_max_sets_per_pool;

        uint64_t m_id_count;
        std::vector<vk::DescriptorPool> m_descriptor_pools {};
        std::vector<std::optional<DescriptorSetImpl>> m_descriptor_sets {};
        size_t m_current_pool_index;

        static std::optional<vk::DescriptorSet> try_create(
            const vk::DispatchLoaderDynamic& loader,
            vk::DescriptorPool pool,
            vk::Device device,
            vk::DescriptorSetLayout layout);

        vk::DescriptorPool create_pool(
            const vk::DispatchLoaderDynamic& loader,
            vk::Device device,
            vk::DescriptorPoolCreateFlags flags = vk::DescriptorPoolCreateFlags());
    };

    static constexpr const size_t c_max_uniform_value_size = std::max(
        { sizeof(float),
          sizeof(mve::Vector2),
          sizeof(mve::Vector3),
          sizeof(mve::Vector4),
          sizeof(mve::Matrix3),
          sizeof(mve::Matrix4) });
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
    QueueFamilyIndices m_vk_queue_family_indices;
    VmaAllocator m_vma_allocator;
    uint64_t m_resource_handle_count;
    CurrentDrawState m_current_draw_state;
    DescriptorSetAllocator m_descriptor_set_allocator {};
    DepthImage m_depth_image;
    vk::SampleCountFlagBits m_msaa_samples;
    RenderImage m_color_image;

    void bind_descriptor_sets(uint32_t num, const std::array<const DescriptorSet*, 4>& descriptor_sets);

    std::vector<FrameInFlight> m_frames_in_flight;

    std::vector<std::optional<VertexBufferImpl>> m_vertex_buffers_new;

    std::vector<std::optional<IndexBufferImpl>> m_index_buffers_new;
    //    std::unordered_map<size_t, VertexBufferImpl> m_vertex_buffers;

    //    std::unordered_map<siz, IndexBufferImpl> m_index_buffers;

    std::unordered_map<DescriptorSetLayoutHandleImpl, vk::DescriptorSetLayout> m_descriptor_set_layouts;

    std::vector<std::optional<GraphicsPipelineImpl>> m_graphics_pipelines_new {};
    //    std::unordered_map<uint64_t, GraphicsPipelineImpl> m_graphics_pipelines {};

    std::vector<std::optional<GraphicsPipelineLayoutImpl>> m_graphics_pipeline_layouts_new {};
    //    std::unordered_map<GraphicsPipelineLayoutHandleImpl, GraphicsPipelineLayoutImpl> m_graphics_pipeline_layouts
    //    {};

    std::vector<std::optional<FramebufferImpl>> m_framebuffers {};

    std::unordered_map<uint64_t, TextureImpl> m_textures {};

    uint32_t m_deferred_function_id_count;
    std::map<uint32_t, DeferredFunction> m_deferred_functions;
    std::queue<uint32_t> m_wait_frames_deferred_functions {};
    std::queue<std::function<void(vk::CommandBuffer)>> m_command_buffer_deferred_functions {};

    template <typename T>
    inline void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, T value, bool persist)
    {
        static_assert(sizeof(T) <= c_max_uniform_value_size);
        uint64_t handle = uniform_buffer.handle();
        DeferredUniformUpdateData update_data {
            .counter = persist ? c_frames_in_flight : 1,
            .handle = handle,
            .location = location,
            .data = {},
            .data_size = sizeof(T)
        };
        memcpy(update_data.data.data(), &value, sizeof(T));
        m_deferred_uniform_updates.push_back(std::move(update_data));
    }

    struct DeferredUniformUpdateData {
        int counter;
        uint64_t handle;
        UniformLocation location;
        std::array<std::byte, c_max_uniform_value_size> data;
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

    void cleanup_vk_swapchain();

    void cleanup_vk_debug_messenger();

    void recreate_swapchain(const Window& window);

    void recreate_framebuffers();

    Texture create_texture(Image image, vk::ImageView image_view, vk::Sampler sampler, uint32_t mip_levels);

    FramebufferImpl create_framebuffer_impl(
        const vk::DispatchLoaderDynamic& loader, std::optional<std::function<void(void)>> callback);

    void wait_ready();

    DescriptorSetLayoutHandleImpl create_descriptor_set_layout(
        const vk::DispatchLoaderDynamic& loader,
        uint32_t set,
        const Shader& vertex_shader,
        const Shader& fragment_shader);

    size_t create_graphics_pipeline_layout(
        const vk::DispatchLoaderDynamic& loader, const Shader& vertex_shader, const Shader& fragment_shader);

    static vk::SampleCountFlagBits get_max_sample_count(
        const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device);

    static DepthImage create_depth_image(
        const vk::DispatchLoaderDynamic& loader,
        vk::PhysicalDevice physical_device,
        vk::Device device,
        vk::CommandPool pool,
        vk::Queue queue,
        VmaAllocator allocator,
        vk::Extent2D extent,
        vk::SampleCountFlagBits samples);

    static vk::Format find_supported_format(
        const vk::DispatchLoaderDynamic& loader,
        vk::PhysicalDevice physical_device,
        const std::vector<vk::Format>& formats,
        vk::ImageTiling tiling,
        vk::FormatFeatureFlags features);

    static vk::Format find_depth_format(const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device);

    void update_uniform(size_t handle, UniformLocation location, void* data_ptr, size_t size, uint32_t frame_index);

    static bool has_stencil_component(vk::Format format);

    static Image create_image(
        VmaAllocator allocator,
        uint32_t width,
        uint32_t height,
        uint32_t mip_levels,
        vk::SampleCountFlagBits samples,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage,
        bool dedicated);

    void defer_to_all_frames(std::function<void(uint32_t)> func);

    void defer_to_next_frame(std::function<void(uint32_t)> func);

    void defer_after_all_frames(std::function<void(uint32_t)> func);

    void defer_to_command_buffer_front(std::function<void(vk::CommandBuffer)> func);

    static RenderImage create_color_image(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        VmaAllocator allocator,
        vk::Extent2D swapchain_extent,
        vk::Format swapchain_format,
        vk::SampleCountFlagBits samples);

    static void cmd_generate_mipmaps(
        const vk::DispatchLoaderDynamic& loader,
        vk::PhysicalDevice physical_device,
        vk::CommandBuffer command_buffer,
        vk::Image image,
        vk::Format format,
        uint32_t width,
        uint32_t height,
        uint32_t mip_levels);

    static vk::CommandBuffer begin_single_submit(
        const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::CommandPool pool);

    static void end_single_submit(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        vk::CommandPool pool,
        vk::CommandBuffer command_buffer,
        vk::Queue queue);

    static void cmd_transition_image_layout(
        const vk::DispatchLoaderDynamic& loader,
        vk::CommandBuffer command_buffer,
        vk::Image image,
        vk::Format format,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        uint32_t mip_levels);

    static void cmd_copy_buffer_to_image(
        const vk::DispatchLoaderDynamic& loader,
        vk::CommandBuffer command_buffer,
        vk::Buffer buffer,
        vk::Image image,
        uint32_t width,
        uint32_t height);

    static vk::Sampler create_texture_sampler(
        const vk::DispatchLoaderDynamic& loader,
        vk::PhysicalDevice physical_device,
        vk::Device device,
        uint32_t mip_levels);

    static vk::ImageView create_image_view(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        vk::Image image,
        vk::Format format,
        vk::ImageAspectFlags aspect_flags,
        uint32_t mip_levels);

    static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
        VkDebugUtilsMessageTypeFlagsEXT msg_type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data);

    static bool has_validation_layer_support(const vk::DispatchLoaderDynamic& loader);

    static vk::Instance create_vk_instance(
        const std::string& app_name, int app_version_major, int app_version_minor, int app_version_patch);

    static vk::DebugUtilsMessengerEXT create_vk_debug_messenger(vk::Instance instance);

    static std::vector<const char*> get_vk_validation_layer_exts();

    static std::vector<const char*> get_vk_device_required_exts();

    static std::vector<const char*> get_vk_instance_required_exts();

    static vk::PhysicalDevice pick_vk_physical_device(
        vk::Instance instance, const vk::DispatchLoaderDynamic& loader, vk::SurfaceKHR surface);

    static bool is_vk_physical_device_suitable(
        const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

    static vk::Device create_vk_logical_device(
        const vk::DispatchLoaderDynamic& loader,
        vk::PhysicalDevice physical_device,
        QueueFamilyIndices queue_family_indices);

    static QueueFamilyIndices get_vk_queue_family_indices(
        const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

    static vk::SurfaceKHR create_vk_surface(vk::Instance instance, GLFWwindow* window);

    static SwapchainSupportDetails get_vk_swapchain_support_details(
        const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

    static vk::SurfaceFormatKHR choose_vk_swapchain_surface_format(
        const std::vector<vk::SurfaceFormatKHR>& available_formats);

    static vk::PresentModeKHR choose_vk_swapchain_present_mode(
        const std::vector<vk::PresentModeKHR>& available_present_modes);

    static vk::Extent2D get_vk_swapchain_extent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    static vk::SwapchainKHR create_vk_swapchain(
        const vk::DispatchLoaderDynamic& loader,
        vk::PhysicalDevice physical_device,
        vk::Device device,
        vk::SurfaceKHR surface,
        vk::SurfaceFormatKHR surface_format,
        vk::Extent2D surface_extent,
        QueueFamilyIndices queue_family_indices);

    static std::vector<vk::Image> get_vk_swapchain_images(
        const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::SwapchainKHR swapchain);

    static std::vector<vk::ImageView> create_vk_swapchain_image_views(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        const std::vector<vk::Image>& swapchain_images,
        vk::Format image_format);

    static vk::Pipeline create_vk_graphics_pipeline(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        const Shader& vertex_shader,
        const Shader& fragment_shader,
        vk::PipelineLayout pipeline_layout,
        vk::RenderPass render_pass,
        const VertexLayout& vertex_layout,
        vk::SampleCountFlagBits samples,
        bool depth_test);

    vk::PipelineLayout create_vk_pipeline_layout(
        const vk::DispatchLoaderDynamic& loader, const std::vector<DescriptorSetLayoutHandleImpl>& layouts);

    static vk::RenderPass create_vk_render_pass(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        vk::Format swapchain_format,
        vk::Format depth_format,
        vk::SampleCountFlagBits samples);

    static vk::RenderPass create_vk_render_pass_framebuffer(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        vk::Format swapchain_format,
        vk::Format depth_format,
        vk::SampleCountFlagBits samples);

    static std::vector<vk::Framebuffer> create_vk_framebuffers(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        const std::vector<vk::ImageView>& swapchain_image_views,
        vk::RenderPass render_pass,
        vk::Extent2D swapchain_extent,
        vk::ImageView color_image_view,
        vk::ImageView depth_image_view);

    static vk::CommandPool create_vk_command_pool(
        const vk::DispatchLoaderDynamic& loader, vk::Device device, QueueFamilyIndices queue_family_indices);

    static std::vector<vk::CommandBuffer> create_vk_command_buffers(
        const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::CommandPool command_pool, int frames_in_flight);

    static Buffer create_buffer(
        VmaAllocator allocator,
        size_t size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memory_usage,
        VmaAllocationCreateFlags flags = 0);

    static void cmd_copy_buffer(
        vk::DispatchLoaderDynamic& loader,
        vk::CommandBuffer command_buffer,
        vk::Buffer src_buffer,
        vk::Buffer dst_buffer,
        vk::DeviceSize size);

    static vk::VertexInputBindingDescription create_vk_binding_description(const VertexLayout& layout);

    static std::vector<vk::VertexInputAttributeDescription> create_vk_attribute_descriptions(
        const VertexLayout& layout);

    static std::vector<FrameInFlight> create_frames_in_flight(
        const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::CommandPool command_pool, int frame_count);

    static vk::DescriptorSetLayout create_vk_descriptor_set_layout(
        const vk::DispatchLoaderDynamic& loader, vk::Device device);

    static vk::DescriptorPool create_vk_descriptor_pool(
        const vk::DispatchLoaderDynamic& loader, vk::Device device, int frames_in_flight);

    static std::vector<vk::DescriptorSet> create_vk_descriptor_sets(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        vk::DescriptorSetLayout layout,
        vk::DescriptorPool pool,
        int count);
};
}

#include "detail/types.hpp"