#include <mve/renderer.hpp>

#include <fstream>
#include <ranges>
#include <set>
#include <utility>
#include <vector>

#include <mve/detail/include_vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <mve/common.hpp>
#include <mve/vertex_data.hpp>
#include <mve/window.hpp>
#include <nnm/nnm.hpp>

#include "descriptor_set_allocator.hpp"
#include "renderer_utils.hpp"
#include "resource_arena.hpp"
#include "structs.hpp"

// ReSharper disable CppUseStructuredBinding
// ReSharper disable CppMemberFunctionMayBeConst

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace mve {

using namespace detail;

struct Renderer::Impl {
    int frames_in_flight_count;
    vk::Instance vk_instance;
    vk::DispatchLoaderDynamic vk_loader;
    vk::DebugUtilsMessengerEXT vk_debug_utils_messenger;
    vk::SurfaceKHR vk_surface;
    vk::PhysicalDevice vk_physical_device;
    vk::Device vk_device;
    vk::Queue vk_graphics_queue;
    vk::Queue vk_present_queue;
    vk::SurfaceFormatKHR vk_swapchain_image_format;
    vk::Extent2D vk_swapchain_extent;
    vk::SwapchainKHR vk_swapchain;
    std::vector<vk::Image> vk_swapchain_images;
    std::vector<vk::ImageView> vk_swapchain_image_views;
    vk::RenderPass vk_render_pass;
    vk::RenderPass vk_render_pass_framebuffer;
    std::vector<vk::Framebuffer> vk_swapchain_framebuffers;
    vk::CommandPool vk_command_pool;
    Msaa max_msaa_samples;
    QueueFamilyIndices queue_family_indices;
    VmaAllocator vma_allocator {};
    CurrentDrawState current_draw_state;
    DescriptorSetAllocator descriptor_set_allocator {};
    DepthImage depth_image;
    vk::SampleCountFlagBits msaa_samples;
    RenderImage color_image;
    std::vector<FrameInFlight> frames_in_flight;
    ResourceArena<VertexBufferImpl> vertex_buffers;
    ResourceArena<IndexBufferImpl> index_buffers;
    ResourceArena<vk::DescriptorSetLayout> descriptor_set_layouts;
    ResourceArena<GraphicsPipelineImpl> graphics_pipelines;
    ResourceArena<GraphicsPipelineLayoutImpl> graphics_pipeline_layouts;
    ResourceArena<FramebufferImpl> framebuffers;
    ResourceArena<TextureImpl> textures;
    std::vector<DeferredOperation> deferred_operations;

    void bind_descriptor_sets(const uint32_t num, const std::array<const DescriptorSet*, 4>& descriptor_sets) const
    {
        {
            std::array<vk::DescriptorSet, 4> sets;
            for (uint32_t i = 0; i < num; i++) {
                sets[i] = frames_in_flight[current_draw_state.frame_index]
                              .descriptor_sets[descriptor_sets[i]->handle()]
                              ->vk_handle;
            }
            MVE_VAL_ASSERT(
                current_draw_state.current_pipeline.has_value(),
                "[Renderer] Cannot bind descriptor set without first binding a graphics pipeline");
            current_draw_state.command_buffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                graphics_pipeline_layouts.get(graphics_pipelines.get(*current_draw_state.current_pipeline).layout)
                    .vk_handle,
                0,
                num,
                sets.data(),
                0,
                nullptr,
                vk_loader);
        }
    }

    template <typename T>
    void update_uniform(
        UniformBuffer& uniform_buffer, const UniformLocation location, T value, const bool persist = true)
    {
        static_assert(sizeof(T) <= max_uniform_value_size);
        const uint64_t handle = uniform_buffer.handle();
        std::array<std::byte, max_uniform_value_size> data; // NOLINT(*-pro-type-member-init)
        memcpy(data.data(), &value, sizeof(T));
        deferred_operations.emplace_back(DeferredUniformUpdateData {
            .counter = persist ? frames_in_flight_count : 1,
            .handle = handle,
            .location = location,
            .data = data,
            .data_size = sizeof(T) });
    }

    void update_uniform(
        const Handle handle,
        const UniformLocation location,
        const void* data_ptr,
        const size_t size,
        const uint32_t frame_index) const
    {
        const UniformBufferImpl& buffer = *frames_in_flight.at(frame_index).uniform_buffers.at(handle);
        memcpy(&buffer.mapped_ptr[location.value()], data_ptr, size);
    }

    void cleanup_vk_swapchain() const
    {
        vk_device.destroy(color_image.vk_image_view, nullptr, vk_loader);
        vmaDestroyImage(vma_allocator, color_image.image.vk_handle, color_image.image.vma_allocation);

        vk_device.destroy(depth_image.vk_image_view, nullptr, vk_loader);
        vmaDestroyImage(vma_allocator, depth_image.image.vk_handle, depth_image.image.vma_allocation);

        for (const vk::Framebuffer framebuffer : vk_swapchain_framebuffers) {
            vk_device.destroy(framebuffer, nullptr, vk_loader);
        }
        for (const vk::ImageView image_view : vk_swapchain_image_views) {
            vk_device.destroy(image_view, nullptr, vk_loader);
        }
        vk_device.destroy(vk_swapchain, nullptr, vk_loader);
    }

    void cleanup_vk_debug_messenger() const
    {
        if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT"));
            func != nullptr) {
            func(vk_instance, static_cast<VkDebugUtilsMessengerEXT>(vk_debug_utils_messenger), nullptr);
        }
    }

    void recreate_swapchain(Renderer& renderer, const Window& window)
    {
        nnm::Vector2i window_size;
        glfwGetFramebufferSize(window.glfw_handle(), &window_size.x, &window_size.y);

        while (window_size == nnm::Vector2i(0, 0)) {
            glfwGetFramebufferSize(window.glfw_handle(), &window_size.x, &window_size.y);
            Window::wait_for_events();
        }

        const vk::Result wait_result = vk_device.waitIdle(vk_loader);
        MVE_ASSERT(wait_result == vk::Result::eSuccess, "[Renderer] Failed to wait idle for swapchain recreation")

        cleanup_vk_swapchain();

        auto [capabilities, formats, present_modes]
            = get_vk_swapchain_support_details(vk_loader, vk_physical_device, vk_surface);

        vk_swapchain_extent = get_vk_swapchain_extent(capabilities, window.glfw_handle());

        vk_swapchain = create_vk_swapchain(
            vk_loader,
            vk_physical_device,
            vk_device,
            vk_surface,
            vk_swapchain_image_format,
            vk_swapchain_extent,
            queue_family_indices);

        vk_swapchain_images = get_vk_swapchain_images(vk_loader, vk_device, vk_swapchain);

        vk_swapchain_image_views = create_vk_swapchain_image_views(
            vk_loader, vk_device, vk_swapchain_images, vk_swapchain_image_format.format);

        color_image = create_color_image(
            vk_loader, vk_device, vma_allocator, vk_swapchain_extent, vk_swapchain_image_format.format, msaa_samples);

        depth_image = create_depth_image(
            vk_loader, vk_physical_device, vk_device, vma_allocator, vk_swapchain_extent, msaa_samples);

        vk_swapchain_framebuffers = create_vk_framebuffers(
            vk_loader,
            vk_device,
            vk_swapchain_image_views,
            vk_render_pass,
            vk_swapchain_extent,
            color_image.vk_image_view,
            depth_image.vk_image_view,
            msaa_samples);

        recreate_framebuffers(renderer);
    }

    void recreate_framebuffers(Renderer& renderer)
    {
        for (Handle i = 0; i < framebuffers.data().size(); i++) {
            if (framebuffers.data()[i].has_value()) {
                for (const vk::Framebuffer& buffer : framebuffers.get(i).vk_framebuffers) {
                    vk_device.destroy(buffer, nullptr, vk_loader);
                }
                framebuffers.get(i) = std::move(create_framebuffer_impl(renderer, vk_loader));
            }
        }
        for (const std::optional<FramebufferImpl>& framebuffer : framebuffers.data()) {
            std::invoke(*framebuffer->callback);
        }
    }

    void recreate_render_passes()
    {
        vk_device.destroy(vk_render_pass, nullptr, vk_loader);
        vk_device.destroy(vk_render_pass_framebuffer, nullptr, vk_loader);

        vk_render_pass = create_vk_render_pass(
            vk_loader,
            vk_device,
            vk_swapchain_image_format.format,
            find_depth_format(vk_loader, vk_physical_device),
            msaa_samples);

        vk_render_pass_framebuffer = create_vk_render_pass_framebuffer(
            vk_loader,
            vk_device,
            vk_swapchain_image_format.format,
            find_depth_format(vk_loader, vk_physical_device),
            msaa_samples);
    }

    void recreate_graphics_pipelines()
    {
        for (std::optional<GraphicsPipelineImpl>& pipeline : graphics_pipelines.data()) {
            if (pipeline.has_value()) {
                vk_device.destroy(pipeline->pipeline, nullptr, vk_loader);

                pipeline->pipeline = create_vk_graphics_pipeline(
                    vk_loader,
                    vk_device,
                    pipeline->vertex_shader,
                    pipeline->fragment_shader,
                    graphics_pipeline_layouts.get(pipeline->layout).vk_handle,
                    vk_render_pass,
                    pipeline->vertex_layout,
                    msaa_samples,
                    pipeline->depth_test);
            }
        }
    }

    Texture create_texture(
        Renderer& renderer,
        const Image& image,
        const vk::ImageView image_view,
        const vk::Sampler sampler,
        const uint32_t mip_levels)
    {

        Handle handle = textures.allocate(TextureImpl {
            .image = image, .vk_image_view = image_view, .vk_sampler = sampler, .mip_levels = mip_levels });

        Logger::debug(std::format("[Renderer] Texture created with ID: {}", handle));

        return { renderer, handle };
    }

    FramebufferImpl create_framebuffer_impl(Renderer& renderer, const vk::DispatchLoaderDynamic& loader)
    {
        auto [image, vk_image_view] = create_color_image(
            vk_loader,
            vk_device,
            vma_allocator,
            vk_swapchain_extent,
            vk_swapchain_image_format.format,
            vk::SampleCountFlagBits::e1);

        std::vector<vk::Framebuffer> framebuffers;
        framebuffers.reserve(vk_swapchain_framebuffers.size());

        for (size_t i = 0; i < vk_swapchain_framebuffers.size(); i++) {

            std::array<vk::ImageView, 3> attachments;
            if (msaa_samples == vk::SampleCountFlagBits::e1) {
                attachments = { vk_image_view, depth_image.vk_image_view };
            }
            else {
                attachments = { color_image.vk_image_view, depth_image.vk_image_view, vk_image_view };
            }

            auto framebuffer_info
                = vk::FramebufferCreateInfo()
                      .setRenderPass(vk_render_pass)
                      .setAttachmentCount(msaa_samples == vk::SampleCountFlagBits::e1 ? 2 : 3)
                      .setPAttachments(attachments.data())
                      .setWidth(vk_swapchain_extent.width)
                      .setHeight(vk_swapchain_extent.height)
                      .setLayers(1);

            vk::ResultValue<vk::Framebuffer> framebuffer_result
                = vk_device.createFramebuffer(framebuffer_info, nullptr, loader);
            MVE_ASSERT(framebuffer_result.result == vk::Result::eSuccess, "[Renderer] Failed to create framebuffer")
            framebuffers.push_back(framebuffer_result.value);
        }

        const vk::Sampler sampler = create_texture_sampler(vk_loader, vk_physical_device, vk_device, 1);

        Texture texture = create_texture(renderer, image, vk_image_view, sampler, 1);

        FramebufferImpl framebuffer_impl {
            .vk_framebuffers = std::move(framebuffers),
            .texture = std::move(texture),
            .callback = std::nullopt,
            .size = { static_cast<int>(vk_swapchain_extent.width), static_cast<int>(vk_swapchain_extent.height) }
        };

        return framebuffer_impl;
    }

    void wait_ready() const
    {

        const FrameInFlight& frame = frames_in_flight[current_draw_state.frame_index];
        const vk::Result fence_wait_result
            = vk_device.waitForFences(frame.in_flight_fence, true, UINT64_MAX, vk_loader);
        MVE_ASSERT(fence_wait_result == vk::Result::eSuccess, "[Renderer] Failed waiting for frame (fences)")
    }

    Handle create_descriptor_set_layout(
        const vk::DispatchLoaderDynamic& loader,
        const uint32_t set,
        const Shader& vertex_shader,
        const Shader& fragment_shader)
    {
        std::array<vk::DescriptorSetLayoutBinding, 2> bindings;
        size_t bindings_count = 0;
        if (vertex_shader.has_descriptor_set(set)) {
            for (const ShaderDescriptorSet& vertex_set = vertex_shader.descriptor_set(set);
                 const auto& shader_binding : vertex_set.bindings() | std::views::values) {
                auto binding = vk::DescriptorSetLayoutBinding()
                                   .setBinding(shader_binding.binding())
                                   .setDescriptorCount(1)
                                   .setPImmutableSamplers(nullptr);

                switch (shader_binding.type()) {
                case ShaderDescriptorType::uniform_buffer:
                    binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
                    binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
                    break;
                case ShaderDescriptorType::combined_image_sampler:
                    binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
                    binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
                    break;
                }
                bindings[bindings_count++] = binding;
            }
        }
        if (fragment_shader.has_descriptor_set(set)) {
            for (const ShaderDescriptorSet& fragment_set = fragment_shader.descriptor_set(set);
                 const auto& fragment_binding : fragment_set.bindings() | std::views::values) {
                auto binding = vk::DescriptorSetLayoutBinding()
                                   .setBinding(fragment_binding.binding())
                                   .setDescriptorCount(1)
                                   .setPImmutableSamplers(nullptr);

                switch (fragment_binding.type()) {
                case ShaderDescriptorType::uniform_buffer:
                    binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
                    binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
                    break;
                case ShaderDescriptorType::combined_image_sampler:
                    binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
                    binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
                    break;
                }
                bindings[bindings_count++] = binding;
            }
        }

        const auto layout_info = vk::DescriptorSetLayoutCreateInfo()
                                     .setPBindings(bindings.data())
                                     .setBindingCount(static_cast<uint32_t>(bindings_count));

        const vk::ResultValue<vk::DescriptorSetLayout> descriptor_set_layout_result
            = vk_device.createDescriptorSetLayout(layout_info, nullptr, loader);
        MVE_ASSERT(
            descriptor_set_layout_result.result == vk::Result::eSuccess,
            "[Renderer] Failed to create descriptor set layout")

        Handle handle = descriptor_set_layouts.allocate(descriptor_set_layout_result.value);

        Logger::debug(std::format("[Renderer] Descriptor set layout created with ID: {}", handle));

        return handle;
    }

    Handle create_graphics_pipeline_layout(
        const vk::DispatchLoaderDynamic& loader, const Shader& vertex_shader, const Shader& fragment_shader)
    {
        std::array<Handle, 4> layouts {};
        size_t layouts_count = 0;
        std::unordered_map<Handle, Handle> descriptor_set_layouts;

        for (uint32_t i = 0; i < 4; i++) {
            if (vertex_shader.has_descriptor_set(i) || fragment_shader.has_descriptor_set(i)) {
                Handle descriptor_set_layout = create_descriptor_set_layout(loader, i, vertex_shader, fragment_shader);
                layouts[layouts_count++] = descriptor_set_layout;
                descriptor_set_layouts.insert({ i, descriptor_set_layout });
            }
        }

        const vk::PipelineLayout vk_layout
            = create_vk_pipeline_layout(vk_loader, std::span(layouts.begin(), layouts_count));

        Handle handle = graphics_pipeline_layouts.allocate(GraphicsPipelineLayoutImpl {
            .vk_handle = vk_layout, .descriptor_set_layouts = std::move(descriptor_set_layouts) });

        Logger::debug(std::format("[Renderer] Graphics pipeline layout created with ID: {}", handle));

        return handle;
    }

    [[nodiscard]] vk::PipelineLayout create_vk_pipeline_layout(
        const vk::DispatchLoaderDynamic& loader, std::span<const Handle> layouts) const
    {
        std::array<vk::DescriptorSetLayout, 4> vk_layouts {};
        size_t vk_layouts_count = 0;
        for (const Handle handle : layouts) {
            vk_layouts[vk_layouts_count++] = descriptor_set_layouts.get(handle);
        }

        const auto pipeline_layout_info
            = vk::PipelineLayoutCreateInfo()
                  .setPSetLayouts(vk_layouts.data())
                  .setSetLayoutCount(static_cast<uint32_t>(vk_layouts_count))
                  .setPushConstantRangeCount(0)
                  .setPPushConstantRanges(nullptr);

        const vk::ResultValue<vk::PipelineLayout> pipeline_layout_result
            = vk_device.createPipelineLayout(pipeline_layout_info, nullptr, loader);
        MVE_ASSERT(pipeline_layout_result.result == vk::Result::eSuccess, "[Renderer] Failed to create pipeline layout")
        return pipeline_layout_result.value;
    }
};

Renderer::Renderer(
    const Window& window, const std::string& app_name, const Version version, const DeviceType preferred_device_type)
{
    int frames_in_flight_count = 2;
    vk::Instance vk_instance = create_vk_instance(app_name, version.major, version.minor, version.patch);
    const auto vkGetDeviceProcAddr
        = reinterpret_cast<PFN_vkGetDeviceProcAddr>(vkGetInstanceProcAddr(vk_instance, "vkGetDeviceProcAddr"));
    vk::DispatchLoaderDynamic vk_loader { vk_instance, vkGetInstanceProcAddr };
    vk::DebugUtilsMessengerEXT vk_debug_utils_messenger;
#ifdef MVE_ENABLE_VALIDATION
    vk_debug_utils_messenger = create_vk_debug_messenger(vk_instance);
#endif
    vk::SurfaceKHR vk_surface = create_vk_surface(vk_instance, window.glfw_handle());
    vk::PhysicalDevice vk_physical_device
        = pick_vk_physical_device(vk_instance, vk_loader, vk_surface, preferred_device_type);
    auto msaa_samples = vk::SampleCountFlagBits::e1;
    QueueFamilyIndices queue_family_indices = get_vk_queue_family_indices(vk_loader, vk_physical_device, vk_surface);
    vk::Device vk_device = create_vk_logical_device(vk_loader, vk_physical_device, queue_family_indices);
    volkLoadDevice(vk_device);
    vk_loader = vk::DispatchLoaderDynamic { vk_instance, vkGetInstanceProcAddr, vk_device, vkGetDeviceProcAddr };

    Msaa max_msaa_samples = vk_samples_to_msaa(get_max_sample_count(vk_loader, vk_physical_device));

    auto [capabilities, formats, present_modes]
        = get_vk_swapchain_support_details(vk_loader, vk_physical_device, vk_surface);

    vk::SurfaceFormatKHR vk_swapchain_image_format = choose_vk_swapchain_surface_format(formats);
    vk::Extent2D vk_swapchain_extent = get_vk_swapchain_extent(capabilities, window.glfw_handle());
    vk::SwapchainKHR vk_swapchain = create_vk_swapchain(
        vk_loader,
        vk_physical_device,
        vk_device,
        vk_surface,
        vk_swapchain_image_format,
        vk_swapchain_extent,
        queue_family_indices);

    std::vector<vk::Image> vk_swapchain_images = get_vk_swapchain_images(vk_loader, vk_device, vk_swapchain);

    std::vector<vk::ImageView> vk_swapchain_image_views
        = create_vk_swapchain_image_views(vk_loader, vk_device, vk_swapchain_images, vk_swapchain_image_format.format);

    vk::Queue vk_graphics_queue = vk_device.getQueue(queue_family_indices.graphics_family.value(), 0, vk_loader);
    vk::Queue vk_present_queue = vk_device.getQueue(queue_family_indices.present_family.value(), 0, vk_loader);

    VmaAllocatorCreateInfo allocatorCreateInfo {};
    VmaVulkanFunctions func {};
    func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    allocatorCreateInfo.pVulkanFunctions = &func;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
    allocatorCreateInfo.physicalDevice = vk_physical_device;
    allocatorCreateInfo.device = vk_device;
    allocatorCreateInfo.instance = vk_instance;
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

    VmaAllocator vma_allocator;
    vmaCreateAllocator(&allocatorCreateInfo, &vma_allocator);

    vk::CommandPool vk_command_pool = create_vk_command_pool(vk_loader, vk_device, queue_family_indices);

    RenderImage color_image = create_color_image(
        vk_loader, vk_device, vma_allocator, vk_swapchain_extent, vk_swapchain_image_format.format, msaa_samples);
    DepthImage depth_image = create_depth_image(
        vk_loader, vk_physical_device, vk_device, vma_allocator, vk_swapchain_extent, msaa_samples);

    vk::RenderPass vk_render_pass = create_vk_render_pass(
        vk_loader,
        vk_device,
        vk_swapchain_image_format.format,
        find_depth_format(vk_loader, vk_physical_device),
        msaa_samples);

    vk::RenderPass vk_render_pass_framebuffer = create_vk_render_pass_framebuffer(
        vk_loader,
        vk_device,
        vk_swapchain_image_format.format,
        find_depth_format(vk_loader, vk_physical_device),
        msaa_samples);

    std::vector<vk::Framebuffer> vk_swapchain_framebuffers = create_vk_framebuffers(
        vk_loader,
        vk_device,
        vk_swapchain_image_views,
        vk_render_pass,
        vk_swapchain_extent,
        color_image.vk_image_view,
        depth_image.vk_image_view,
        msaa_samples);

    auto frames_in_flight = create_frames_in_flight(vk_loader, vk_device, vk_command_pool, frames_in_flight_count);

    CurrentDrawState current_draw_state {
        .is_drawing = false,
        .image_index = 0,
        .command_buffer = nullptr,
        .current_pipeline = std::nullopt,
        .frame_index = 0
    };

    m_impl = std::make_unique<Impl>(Impl {
        .frames_in_flight_count = frames_in_flight_count,
        .vk_instance = vk_instance,
        .vk_loader = vk_loader,
        .vk_debug_utils_messenger = vk_debug_utils_messenger,
        .vk_surface = vk_surface,
        .vk_physical_device = vk_physical_device,
        .vk_device = vk_device,
        .vk_graphics_queue = vk_graphics_queue,
        .vk_present_queue = vk_present_queue,
        .vk_swapchain_image_format = vk_swapchain_image_format,
        .vk_swapchain_extent = vk_swapchain_extent,
        .vk_swapchain = vk_swapchain,
        .vk_swapchain_images = vk_swapchain_images,
        .vk_swapchain_image_views = vk_swapchain_image_views,
        .vk_render_pass = vk_render_pass,
        .vk_render_pass_framebuffer = vk_render_pass_framebuffer,
        .vk_swapchain_framebuffers = vk_swapchain_framebuffers,
        .vk_command_pool = vk_command_pool,
        .max_msaa_samples = max_msaa_samples,
        .queue_family_indices = queue_family_indices,
        .vma_allocator = vma_allocator,
        .current_draw_state = current_draw_state,
        .depth_image = depth_image,
        .msaa_samples = msaa_samples,
        .color_image = color_image,
        .frames_in_flight = frames_in_flight });
}

Renderer::~Renderer()
{
#ifdef MVE_ENABLE_VALIDATION
    m_impl->cleanup_vk_debug_messenger();
#endif
    // ReSharper disable once CppDFAUnreadVariable
    // ReSharper disable once CppDFAUnusedValue
    vk::Result _ = m_impl->vk_device.waitIdle(m_impl->vk_loader);

    m_impl->cleanup_vk_swapchain();

    for (std::optional<FramebufferImpl>& framebuffer : m_impl->framebuffers.data()) {
        if (framebuffer.has_value()) {
            for (const vk::Framebuffer& buffer : framebuffer->vk_framebuffers) {
                m_impl->vk_device.destroy(buffer, nullptr, m_impl->vk_loader);
            }
            if (framebuffer->texture.is_valid()) {
                destroy(framebuffer->texture);
            }
        }
    }

    for (const std::optional<TextureImpl>& texture : m_impl->textures.data()) {
        if (texture.has_value()) {
            m_impl->vk_device.destroy(texture->vk_sampler, nullptr, m_impl->vk_loader);
            m_impl->vk_device.destroy(texture->vk_image_view, nullptr, m_impl->vk_loader);
            vmaDestroyImage(m_impl->vma_allocator, texture->image.vk_handle, texture->image.vma_allocation);
        }
    }

    m_impl->descriptor_set_allocator.cleanup(m_impl->vk_loader, m_impl->vk_device);

    for (FrameInFlight& frame : m_impl->frames_in_flight) {
        for (std::optional<UniformBufferImpl>& uniform_buffer : frame.uniform_buffers) {
            if (uniform_buffer.has_value()) {
                vmaUnmapMemory(m_impl->vma_allocator, uniform_buffer->buffer.vma_allocation);
                vmaDestroyBuffer(
                    m_impl->vma_allocator, uniform_buffer->buffer.vk_handle, uniform_buffer->buffer.vma_allocation);
            }
        }
    }

    for (const std::optional<vk::DescriptorSetLayout>& layout : m_impl->descriptor_set_layouts.data()) {
        if (layout.has_value()) {
            m_impl->vk_device.destroy(*layout, nullptr, m_impl->vk_loader);
        }
    }

    for (const std::optional<VertexBufferImpl>& vertex_buffer : m_impl->vertex_buffers.data()) {
        if (vertex_buffer.has_value()) {
            vmaDestroyBuffer(
                m_impl->vma_allocator, vertex_buffer->buffer.vk_handle, vertex_buffer->buffer.vma_allocation);
        }
    }

    for (const std::optional<IndexBufferImpl>& index_buffer : m_impl->index_buffers.data()) {
        if (index_buffer.has_value()) {
            vmaDestroyBuffer(
                m_impl->vma_allocator, index_buffer->buffer.vk_handle, index_buffer->buffer.vma_allocation);
        }
    }

    for (const DeferredOperation& operation : m_impl->deferred_operations) {
        if (const auto deferred = std::get_if<DeferredDestroyBuffer>(&operation)) {
            vmaDestroyBuffer(m_impl->vma_allocator, deferred->buffer.vk_handle, deferred->buffer.vma_allocation);
        }
    }

    vmaDestroyAllocator(m_impl->vma_allocator);

    for (const std::optional<GraphicsPipelineImpl>& pipeline : m_impl->graphics_pipelines.data()) {
        if (pipeline.has_value()) {
            m_impl->vk_device.destroy(pipeline->pipeline, nullptr, m_impl->vk_loader);
        }
    }

    for (const std::optional<GraphicsPipelineLayoutImpl>& layout : m_impl->graphics_pipeline_layouts.data()) {
        if (layout.has_value()) {
            m_impl->vk_device.destroy(layout->vk_handle, nullptr, m_impl->vk_loader);
        }
    }

    m_impl->vk_device.destroy(m_impl->vk_render_pass, nullptr, m_impl->vk_loader);
    m_impl->vk_device.destroy(m_impl->vk_render_pass_framebuffer, nullptr, m_impl->vk_loader);

    for (const FrameInFlight& frame : m_impl->frames_in_flight) {
        m_impl->vk_device.destroy(frame.render_finished_semaphore, nullptr, m_impl->vk_loader);
        m_impl->vk_device.destroy(frame.image_available_semaphore, nullptr, m_impl->vk_loader);
        m_impl->vk_device.destroy(frame.in_flight_fence, nullptr, m_impl->vk_loader);
    }

    m_impl->vk_device.destroy(m_impl->vk_command_pool, nullptr, m_impl->vk_loader);

    m_impl->vk_device.destroy(nullptr, m_impl->vk_loader);

    m_impl->vk_instance.destroy(m_impl->vk_surface, nullptr, m_impl->vk_loader);
    m_impl->vk_instance.destroy(nullptr, m_impl->vk_loader);
}

void Renderer::destroy(VertexBuffer& vertex_buffer)
{
    MVE_VAL_ASSERT(vertex_buffer.is_valid(), "[Renderer] Attempted to destroy invalid vertex buffer")
    Logger::debug(std::format("[Renderer] Destroyed vertex buffer with ID: {}", vertex_buffer.handle()));
    const Handle handle = vertex_buffer.handle();
    vertex_buffer.invalidate();
    m_impl->deferred_operations.emplace_back(DeferredDestroyVertexBuffer { handle });
}

void Renderer::begin_render_pass_present(const std::array<float, 4>& clear_color) const
{
    MVE_ASSERT(
        m_impl->current_draw_state.is_drawing,
        "[Renderer] Cannot begin render pass without first calling begin_frame()");

    std::array<vk::ClearValue, 2> clear_values {};
    clear_values[0].setColor(vk::ClearColorValue(clear_color));
    clear_values[1].setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));

    const auto render_pass_begin_info
        = vk::RenderPassBeginInfo()
              .setRenderPass(m_impl->vk_render_pass)
              .setFramebuffer(m_impl->vk_swapchain_framebuffers[m_impl->current_draw_state.image_index])
              .setRenderArea(vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_impl->vk_swapchain_extent))
              // ReSharper disable once CppRedundantCastExpression
              .setClearValueCount(static_cast<uint32_t>(clear_values.size()))
              .setPClearValues(clear_values.data());

    m_impl->current_draw_state.command_buffer.beginRenderPass(
        render_pass_begin_info, vk::SubpassContents::eInline, m_impl->vk_loader);

    auto viewport
        = vk::Viewport()
              .setX(0.0f)
              .setY(0.0f)
              .setWidth(static_cast<float>(m_impl->vk_swapchain_extent.width))
              .setHeight(static_cast<float>(m_impl->vk_swapchain_extent.height))
              .setMinDepth(0.0f)
              .setMaxDepth(1.0f);

    m_impl->current_draw_state.command_buffer.setViewport(0, { viewport }, m_impl->vk_loader);

    auto scissor = vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_impl->vk_swapchain_extent);

    m_impl->current_draw_state.command_buffer.setScissor(0, { scissor }, m_impl->vk_loader);
}

void Renderer::begin_render_pass_framebuffer(const Framebuffer& framebuffer) const
{
    MVE_ASSERT(
        m_impl->current_draw_state.is_drawing,
        "[Renderer] Cannot begin render pass without first calling begin_frame()");

    constexpr auto clear_color = vk::ClearColorValue(std::array { 0.0f, 0.0f, 0.0f, 1.0f });

    std::array<vk::ClearValue, 2> clear_values {};
    clear_values[0].setColor(clear_color);
    clear_values[1].setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));

    const auto render_pass_begin_info
        = vk::RenderPassBeginInfo()
              .setRenderPass(m_impl->vk_render_pass_framebuffer)
              .setFramebuffer(m_impl->framebuffers.get(framebuffer.handle())
                                  .vk_framebuffers[m_impl->current_draw_state.image_index])
              .setRenderArea(vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_impl->vk_swapchain_extent))
              // ReSharper disable once CppRedundantCastExpression
              .setClearValueCount(static_cast<uint32_t>(clear_values.size()))
              .setPClearValues(clear_values.data());

    m_impl->current_draw_state.command_buffer.beginRenderPass(
        render_pass_begin_info, vk::SubpassContents::eInline, m_impl->vk_loader);

    auto viewport
        = vk::Viewport()
              .setX(0.0f)
              .setY(0.0f)
              .setWidth(static_cast<float>(m_impl->vk_swapchain_extent.width))
              .setHeight(static_cast<float>(m_impl->vk_swapchain_extent.height))
              .setMinDepth(0.0f)
              .setMaxDepth(1.0f);

    m_impl->current_draw_state.command_buffer.setViewport(0, { viewport }, m_impl->vk_loader);

    auto scissor = vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_impl->vk_swapchain_extent);

    m_impl->current_draw_state.command_buffer.setScissor(0, { scissor }, m_impl->vk_loader);
}

void Renderer::begin_frame(const Window& window)
{
    MVE_VAL_ASSERT(
        !m_impl->current_draw_state.is_drawing,
        "[Renderer] Cannot begin frame before first calling end_frame() on previous frame");

    m_impl->current_draw_state.is_drawing = true;

    FrameInFlight& frame = m_impl->frames_in_flight[m_impl->current_draw_state.frame_index];

    m_impl->wait_ready();

    const vk::ResultValue<uint32_t> acquire_result = m_impl->vk_device.acquireNextImageKHR(
        m_impl->vk_swapchain, UINT64_MAX, frame.image_available_semaphore, nullptr, m_impl->vk_loader);
    if (acquire_result.result == vk::Result::eSuboptimalKHR) {
        m_impl->recreate_swapchain(*this, window);
        m_impl->current_draw_state.is_drawing = false;
        return;
    }
    MVE_ASSERT(acquire_result.result == vk::Result::eSuccess, "[Renderer] Failed to acquire swapchain image")
    m_impl->current_draw_state.image_index = acquire_result.value;

    vmaSetCurrentFrameIndex(m_impl->vma_allocator, acquire_result.value);

    // ReSharper disable once CppExpressionWithoutSideEffects
    m_impl->vk_device.resetFences({ frame.in_flight_fence }, m_impl->vk_loader);

    frame.command_buffer.reset(vk::CommandBufferResetFlags(), m_impl->vk_loader);

    auto it = m_impl->deferred_operations.begin();
    while (it != m_impl->deferred_operations.end()) {
        if (const auto deferred = std::get_if<DeferredDestroyBuffer>(&*it);
            deferred != nullptr && deferred->frame_index == m_impl->current_draw_state.frame_index) {
            vmaDestroyBuffer(m_impl->vma_allocator, deferred->buffer.vk_handle, deferred->buffer.vma_allocation);
            it = m_impl->deferred_operations.erase(it);
        }
        else {
            ++it;
        }
    }

    it = m_impl->deferred_operations.begin();
    while (it != m_impl->deferred_operations.end()) {
        if (const auto deferred_descriptor = std::get_if<DeferredDescriptorWriteData>(&*it)) {
            auto& [counter, data_type, data_handle, descriptor_handle, binding] = *deferred_descriptor;
            vk::DescriptorBufferInfo buffer_info;
            vk::DescriptorImageInfo image_info;
            vk::WriteDescriptorSet descriptor_write;
            switch (data_type) {
            case DescriptorBindingType::uniform_buffer:
                buffer_info
                    = vk::DescriptorBufferInfo()
                          .setBuffer(m_impl->frames_in_flight[m_impl->current_draw_state.frame_index]
                                         .uniform_buffers[data_handle]
                                         ->buffer.vk_handle)
                          .setOffset(0)
                          .setRange(m_impl->frames_in_flight[m_impl->current_draw_state.frame_index]
                                        .uniform_buffers[data_handle]
                                        ->size);

                descriptor_write
                    = vk::WriteDescriptorSet()
                          .setDstSet(m_impl->frames_in_flight[m_impl->current_draw_state.frame_index]
                                         .descriptor_sets[descriptor_handle]
                                         ->vk_handle)
                          .setDstBinding(binding)
                          .setDstArrayElement(0)
                          .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                          .setDescriptorCount(1)
                          .setPBufferInfo(&buffer_info);

                m_impl->vk_device.updateDescriptorSets(1, &descriptor_write, 0, nullptr, m_impl->vk_loader);
                break;
            case DescriptorBindingType::texture:
                image_info = vk::DescriptorImageInfo()
                                 .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                                 .setImageView(m_impl->textures.get(data_handle).vk_image_view)
                                 .setSampler(m_impl->textures.get(data_handle).vk_sampler);

                descriptor_write
                    = vk::WriteDescriptorSet()
                          .setDstSet(m_impl->frames_in_flight[m_impl->current_draw_state.frame_index]
                                         .descriptor_sets[descriptor_handle]
                                         ->vk_handle)
                          .setDstBinding(binding)
                          .setDstArrayElement(0)
                          .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                          .setDescriptorCount(1)
                          .setPImageInfo(&image_info);

                m_impl->vk_device.updateDescriptorSets(1, &descriptor_write, 0, nullptr, m_impl->vk_loader);
                break;
            }
            counter--;
            if (counter <= 0) {
                it = m_impl->deferred_operations.erase(it);
            }
            else {
                ++it;
            }
        }
        else if (const auto deferred_uniform = std::get_if<DeferredUniformUpdateData>(&*it)) {
            auto& [counter, handle, location, data, data_size] = *deferred_uniform;
            m_impl->update_uniform(handle, location, data.data(), data_size, m_impl->current_draw_state.frame_index);
            counter--;
            if (counter <= 0) {
                it = m_impl->deferred_operations.erase(it);
            }
            else {
                ++it;
            }
        }
        else if (const auto deferred_destroy_vertex_buffer = std::get_if<DeferredDestroyVertexBuffer>(&*it)) {
            if (deferred_destroy_vertex_buffer->frame_count > m_impl->frames_in_flight_count) {
                vmaDestroyBuffer(
                    m_impl->vma_allocator,
                    m_impl->vertex_buffers.get(deferred_destroy_vertex_buffer->handle).buffer.vk_handle,
                    m_impl->vertex_buffers.get(deferred_destroy_vertex_buffer->handle).buffer.vma_allocation);
                m_impl->vertex_buffers.free(deferred_destroy_vertex_buffer->handle);
                it = m_impl->deferred_operations.erase(it);
            }
            else {
                deferred_destroy_vertex_buffer->frame_count++;
                ++it;
            }
        }
        else if (const auto deferred_destroy_texture = std::get_if<DeferredDestroyTexture>(&*it)) {
            if (deferred_destroy_texture->frame_count > m_impl->frames_in_flight_count) {
                auto& [image, vk_image_view, vk_sampler, mip_levels]
                    = m_impl->textures.get(deferred_destroy_texture->handle);
                m_impl->vk_device.destroy(vk_sampler, nullptr, m_impl->vk_loader);
                m_impl->vk_device.destroy(vk_image_view, nullptr, m_impl->vk_loader);
                vmaDestroyImage(m_impl->vma_allocator, image.vk_handle, image.vma_allocation);
                m_impl->textures.free(deferred_destroy_texture->handle);
                it = m_impl->deferred_operations.erase(it);
            }
            else {
                deferred_destroy_texture->frame_count++;
                ++it;
            }
        }
        else if (const auto deferred_destroy_descriptor_set = std::get_if<DeferredDestroyDescriptorSet>(&*it)) {
            if (deferred_destroy_descriptor_set->frame_count > m_impl->frames_in_flight_count) {
                for (FrameInFlight& f : m_impl->frames_in_flight) {
                    const DescriptorSetImpl set = *f.descriptor_sets[deferred_destroy_descriptor_set->handle];
                    f.descriptor_sets[deferred_destroy_descriptor_set->handle].reset();
                    m_impl->descriptor_set_allocator.free(m_impl->vk_loader, m_impl->vk_device, set);
                }
                it = m_impl->deferred_operations.erase(it);
            }
            else {
                deferred_destroy_descriptor_set->frame_count++;
                ++it;
            }
        }
        else if (const auto deferred_destroy_graphics_pipeline = std::get_if<DeferredDestroyGraphicsPipeline>(&*it)) {
            if (deferred_destroy_graphics_pipeline->frame_count > m_impl->frames_in_flight_count) {
                // Descriptor set layouts
                for (auto& set_layout :
                     m_impl->graphics_pipeline_layouts
                             .get(m_impl->graphics_pipelines.get(deferred_destroy_graphics_pipeline->handle).layout)
                             .descriptor_set_layouts
                         | std::views::values) {
                    m_impl->vk_device.destroy(
                        m_impl->descriptor_set_layouts.get(set_layout), nullptr, m_impl->vk_loader);
                    m_impl->descriptor_set_layouts.free(set_layout);
                }

                // Pipeline layout
                m_impl->vk_device.destroy(
                    m_impl->graphics_pipeline_layouts
                        .get(m_impl->graphics_pipelines.get(deferred_destroy_graphics_pipeline->handle).layout)
                        .vk_handle,
                    nullptr,
                    m_impl->vk_loader);
                m_impl->graphics_pipeline_layouts.free(
                    m_impl->graphics_pipelines.get(deferred_destroy_graphics_pipeline->handle).layout);

                // Graphics pipeline
                m_impl->vk_device.destroy(
                    m_impl->graphics_pipelines.get(deferred_destroy_graphics_pipeline->handle).pipeline,
                    nullptr,
                    m_impl->vk_loader);
                m_impl->graphics_pipelines.free(deferred_destroy_graphics_pipeline->handle);
                it = m_impl->deferred_operations.erase(it);
            }
            else {
                deferred_destroy_graphics_pipeline->frame_count++;
                ++it;
            }
        }
        else if (const auto deferred_destroy_uniform_buffer = std::get_if<DeferredDestroyUniformBuffer>(&*it)) {
            if (deferred_destroy_uniform_buffer->frame_count > m_impl->frames_in_flight_count) {

                for (const FrameInFlight& f : m_impl->frames_in_flight) {
                    auto [buffer, size, mapped_ptr] = *f.uniform_buffers.at(deferred_destroy_uniform_buffer->handle);
                    vmaUnmapMemory(m_impl->vma_allocator, buffer.vma_allocation);
                    vmaDestroyBuffer(m_impl->vma_allocator, buffer.vk_handle, buffer.vma_allocation);
                }

                for (FrameInFlight& f : m_impl->frames_in_flight) {
                    f.uniform_buffers[deferred_destroy_uniform_buffer->handle].reset();
                }
                it = m_impl->deferred_operations.erase(it);
            }
            else {
                deferred_destroy_uniform_buffer->frame_count++;
                ++it;
            }
        }
        else if (const auto deferred_destroy_index_buffer = std::get_if<DeferredDestroyIndexBuffer>(&*it)) {
            if (deferred_destroy_index_buffer->frame_count > m_impl->frames_in_flight_count) {
                vmaDestroyBuffer(
                    m_impl->vma_allocator,
                    m_impl->index_buffers.get(deferred_destroy_index_buffer->handle).buffer.vk_handle,
                    m_impl->index_buffers.get(deferred_destroy_index_buffer->handle).buffer.vma_allocation);
                m_impl->index_buffers.free(deferred_destroy_index_buffer->handle);
                it = m_impl->deferred_operations.erase(it);
            }
            else {
                deferred_destroy_index_buffer->frame_count++;
                ++it;
            }
        }
        else if (const auto deferred_destroy_framebuffer = std::get_if<DeferredDestroyFramebuffer>(&*it)) {
            if (deferred_destroy_framebuffer->frame_count > m_impl->frames_in_flight_count) {
                auto& [vk_framebuffers, texture, callback, size]
                    = m_impl->framebuffers.get(deferred_destroy_framebuffer->handle);
                m_impl->textures.free(deferred_destroy_framebuffer->handle);
                for (const vk::Framebuffer& buffer : vk_framebuffers) {
                    m_impl->vk_device.destroy(buffer, nullptr, m_impl->vk_loader);
                }
                m_impl->framebuffers.free(deferred_destroy_framebuffer->handle);
                it = m_impl->deferred_operations.erase(it);
            }
            else {
                deferred_destroy_framebuffer->frame_count++;
                ++it;
            }
        }
        else {
            ++it;
        }
    }

    m_impl->current_draw_state.command_buffer
        = m_impl->frames_in_flight[m_impl->current_draw_state.frame_index].command_buffer;

    constexpr auto buffer_begin_info = vk::CommandBufferBeginInfo();
    const vk::Result begin_result
        = m_impl->current_draw_state.command_buffer.begin(buffer_begin_info, m_impl->vk_loader);
    MVE_ASSERT(begin_result == vk::Result::eSuccess, "[Renderer] Failed to begin command buffer recording")

    thread_local std::vector<DeferredOperation> next_deferred;
    next_deferred.clear();
    for (const DeferredOperation& deferred : m_impl->deferred_operations) {
        if (const auto deferred_copy = std::get_if<DeferredCopyStagingBuffer>(&deferred)) {
            cmd_copy_buffer(
                m_impl->vk_loader,
                m_impl->current_draw_state.command_buffer,
                deferred_copy->staging_buffer.vk_handle,
                deferred_copy->dst_buffer.vk_handle,
                deferred_copy->buffer_size);

            const auto barrier
                = vk::BufferMemoryBarrier()
                      .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                      .setDstAccessMask(vk::AccessFlagBits::eVertexAttributeRead)
                      .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                      .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                      .setBuffer(deferred_copy->dst_buffer.vk_handle)
                      .setOffset(0)
                      .setSize(deferred_copy->buffer_size);

            m_impl->current_draw_state.command_buffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eVertexInput,
                {},
                0,
                nullptr,
                1,
                &barrier,
                0,
                nullptr,
                m_impl->vk_loader);
            next_deferred.emplace_back(DeferredDestroyBuffer {
                .buffer = deferred_copy->staging_buffer, .frame_index = m_impl->current_draw_state.frame_index });
        }
        else if (const auto deferred_image = std::get_if<DeferredCopyBufferImage>(&deferred)) {
            cmd_transition_image_layout(
                m_impl->vk_loader,
                m_impl->current_draw_state.command_buffer,
                deferred_image->dst_image.vk_handle,
                deferred_image->image_format,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eTransferDstOptimal,
                deferred_image->mip_levels);

            cmd_copy_buffer_to_image(
                m_impl->vk_loader,
                m_impl->current_draw_state.command_buffer,
                deferred_image->staging_buffer.vk_handle,
                deferred_image->dst_image.vk_handle,
                deferred_image->dst_image.width,
                deferred_image->dst_image.height);

            cmd_generate_mipmaps(
                m_impl->vk_loader,
                m_impl->vk_physical_device,
                m_impl->current_draw_state.command_buffer,
                deferred_image->dst_image.vk_handle,
                deferred_image->image_format,
                deferred_image->dst_image.width,
                deferred_image->dst_image.height,
                deferred_image->mip_levels);

            next_deferred.emplace_back(DeferredDestroyBuffer {
                .buffer = deferred_image->staging_buffer, .frame_index = m_impl->current_draw_state.frame_index });
        }
        else {
            next_deferred.emplace_back(deferred);
        }
    }
    std::swap(next_deferred, m_impl->deferred_operations);
}

void Renderer::end_frame(const Window& window)
{
    const vk::Result end_result = m_impl->current_draw_state.command_buffer.end(m_impl->vk_loader);
    MVE_ASSERT(end_result == vk::Result::eSuccess, "[Renderer] Failed to end command buffer recording")

    const FrameInFlight& frame = m_impl->frames_in_flight[m_impl->current_draw_state.frame_index];

    const vk::Semaphore wait_semaphores[] = { frame.image_available_semaphore };
    constexpr vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    const vk::Semaphore signal_semaphores[] = { frame.render_finished_semaphore };

    auto submit_info
        = vk::SubmitInfo()
              .setWaitSemaphores(wait_semaphores)
              .setWaitDstStageMask(wait_stages)
              .setCommandBufferCount(1)
              .setPCommandBuffers(&frame.command_buffer)
              .setSignalSemaphores(signal_semaphores);

    const vk::Result graphics_submit_result
        = m_impl->vk_graphics_queue.submit({ submit_info }, frame.in_flight_fence, m_impl->vk_loader);
    MVE_ASSERT(graphics_submit_result == vk::Result::eSuccess, "[Renderer] Failed to submit to graphics queue")

    const vk::SwapchainKHR swapchains[] = { m_impl->vk_swapchain };

    const auto present_info
        = vk::PresentInfoKHR()
              .setWaitSemaphores(signal_semaphores)
              .setSwapchains(swapchains)
              .setPImageIndices(&m_impl->current_draw_state.image_index);

    if (const vk::Result present_result = m_impl->vk_present_queue.presentKHR(present_info, m_impl->vk_loader);
        present_result == vk::Result::eSuboptimalKHR || present_result == vk::Result::eErrorOutOfDateKHR) {
        m_impl->recreate_swapchain(*this, window);
    }
    else {
        MVE_ASSERT(present_result == vk::Result::eSuccess, "[Renderer] Failed to present frame")
    }

    m_impl->current_draw_state.frame_index
        = (m_impl->current_draw_state.frame_index + 1) % m_impl->frames_in_flight_count;

    m_impl->current_draw_state.is_drawing = false;
}

nnm::Vector2i Renderer::extent() const
{
    return { static_cast<int>(m_impl->vk_swapchain_extent.width),
             static_cast<int>(m_impl->vk_swapchain_extent.height) };
}

void Renderer::resize(const Window& window)
{
    m_impl->recreate_swapchain(*this, window);
}

void Renderer::write_descriptor_binding(
    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    DescriptorSet& descriptor_set,
    const ShaderDescriptorBinding& descriptor_binding,
    const Texture& texture)
{
    m_impl->deferred_operations.emplace_back(DeferredDescriptorWriteData {
        .counter = m_impl->frames_in_flight_count,
        .data_type = DescriptorBindingType::texture,
        .data_handle = texture.handle(),
        .descriptor_handle = descriptor_set.handle(),
        .binding = descriptor_binding.binding() });
}

VertexBuffer Renderer::create_vertex_buffer(const VertexData& vertex_data)
{
    const size_t buffer_size = vertex_layout_bytes(vertex_data.layout()) * vertex_data.vertex_count();

    MVE_VAL_ASSERT(buffer_size != 0, "[Renderer] Attempt to allocate empty vertex buffer")

    const Buffer staging_buffer = create_buffer(
        m_impl->vma_allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* data;
    vmaMapMemory(m_impl->vma_allocator, staging_buffer.vma_allocation, &data);
    memcpy(data, vertex_data.data(), buffer_size);
    vmaUnmapMemory(m_impl->vma_allocator, staging_buffer.vma_allocation);

    const Buffer buffer = create_buffer(
        m_impl->vma_allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

    m_impl->deferred_operations.emplace_back(DeferredCopyStagingBuffer {
        .staging_buffer = staging_buffer, .dst_buffer = buffer, .buffer_size = buffer_size });

    Handle handle = m_impl->vertex_buffers.allocate(
        VertexBufferImpl { .buffer = buffer, .vertex_count = vertex_data.vertex_count() });

    Logger::debug(std::format("[Renderer] Vertex buffer created with ID: {}", handle));

    return { *this, handle };
}

void Renderer::bind_vertex_buffer(const VertexBuffer& vertex_buffer) const
{
    constexpr vk::DeviceSize offset = 0;
    m_impl->current_draw_state.command_buffer.bindVertexBuffers(
        0, 1, &m_impl->vertex_buffers.get(vertex_buffer.handle()).buffer.vk_handle, &offset, m_impl->vk_loader);
}

IndexBuffer Renderer::create_index_buffer(const std::span<const uint32_t> indices)
{
    const size_t buffer_size = sizeof(uint32_t) * indices.size();

    const Buffer staging_buffer = create_buffer(
        m_impl->vma_allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* data;
    vmaMapMemory(m_impl->vma_allocator, staging_buffer.vma_allocation, &data);
    memcpy(data, indices.data(), buffer_size);
    vmaUnmapMemory(m_impl->vma_allocator, staging_buffer.vma_allocation);

    const Buffer buffer = create_buffer(
        m_impl->vma_allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        {});

    m_impl->deferred_operations.emplace_back(DeferredCopyStagingBuffer {
        .staging_buffer = staging_buffer, .dst_buffer = buffer, .buffer_size = buffer_size });

    Handle handle = m_impl->index_buffers.allocate(IndexBufferImpl { .buffer = buffer, .index_count = indices.size() });

    Logger::debug(std::format("[Renderer] Index buffer created with ID: {}", handle));

    return { *this, handle };
}

void Renderer::draw_index_buffer(const IndexBuffer& index_buffer)
{
    auto& [buffer, index_count] = m_impl->index_buffers.get(index_buffer.handle());
    m_impl->current_draw_state.command_buffer.bindIndexBuffer(
        buffer.vk_handle, 0, vk::IndexType::eUint32, m_impl->vk_loader);
    m_impl->current_draw_state.command_buffer.drawIndexed(
        static_cast<uint32_t>(index_count), 1, 0, 0, 0, m_impl->vk_loader);
}

GraphicsPipeline Renderer::create_graphics_pipeline(
    const Shader& vertex_shader,
    const Shader& fragment_shader,
    const std::span<const VertexAttributeType> vertex_layout,
    const bool depth_test)
{
    const Handle layout = m_impl->create_graphics_pipeline_layout(m_impl->vk_loader, vertex_shader, fragment_shader);

    const vk::Pipeline vk_pipeline = create_vk_graphics_pipeline(
        m_impl->vk_loader,
        m_impl->vk_device,
        vertex_shader,
        fragment_shader,
        m_impl->graphics_pipeline_layouts.get(layout).vk_handle,
        m_impl->vk_render_pass,
        vertex_layout,
        m_impl->msaa_samples,
        depth_test);

    Handle handle = m_impl->graphics_pipelines.allocate(GraphicsPipelineImpl {
        .layout = layout,
        .pipeline = vk_pipeline,
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
        .vertex_layout = std::vector(vertex_layout.begin(), vertex_layout.end()),
        .depth_test = depth_test });

    Logger::debug(std::format("[Renderer] Graphics pipeline created with ID: {}", handle));

    return { *this, handle };
}

DescriptorSet Renderer::create_descriptor_set(
    const GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set)
{
    std::vector<DescriptorSetImpl> descriptor_sets;
    descriptor_sets.reserve(m_impl->frames_in_flight_count);

    const vk::DescriptorSetLayout layout = m_impl->descriptor_set_layouts.get(
        m_impl->graphics_pipeline_layouts.get(m_impl->graphics_pipelines.get(graphics_pipeline.handle()).layout)
            .descriptor_set_layouts.at(descriptor_set.set()));

    for (int i = 0; i < m_impl->frames_in_flight_count; i++) {
        descriptor_sets.push_back(
            m_impl->descriptor_set_allocator.create(m_impl->vk_loader, m_impl->vk_device, layout));
    }

    const FrameInFlight& ref_frame = m_impl->frames_in_flight.at(0);
    std::optional<Handle> id;
    for (Handle i = 0; i < ref_frame.descriptor_sets.size(); i++) {
        if (!ref_frame.descriptor_sets[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = ref_frame.descriptor_sets.size();

        for (FrameInFlight& frame : m_impl->frames_in_flight) {
            frame.descriptor_sets.emplace_back();
        }
    }
    int i = 0;

    for (FrameInFlight& frame : m_impl->frames_in_flight) {
        frame.descriptor_sets[*id] = descriptor_sets.at(i);
        i++;
    }

    Logger::debug(std::format("[Renderer] Descriptor set created with ID: {}", *id));

    return { *this, *id };
}

void Renderer::bind_graphics_pipeline(const GraphicsPipeline& graphics_pipeline)
{
    // TODO: Re-evaluate this
    // if (m_impl->current_draw_state.current_pipeline.has_value()
    //     && m_impl->current_draw_state.current_pipeline == graphics_pipeline.handle()) {
    //     return;
    // }
    m_impl->current_draw_state.command_buffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        m_impl->graphics_pipelines.get(graphics_pipeline.handle()).pipeline,
        m_impl->vk_loader);
    m_impl->current_draw_state.current_pipeline = graphics_pipeline.handle();
}

void Renderer::write_descriptor_binding(
    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    DescriptorSet& descriptor_set,
    const ShaderDescriptorBinding& descriptor_binding,
    const UniformBuffer& uniform_buffer)
{
    m_impl->deferred_operations.emplace_back(DeferredDescriptorWriteData {
        .counter = m_impl->frames_in_flight_count,
        .data_type = DescriptorBindingType::uniform_buffer,
        .data_handle = uniform_buffer.handle(),
        .descriptor_handle = descriptor_set.handle(),
        .binding = descriptor_binding.binding() });
}

void Renderer::bind_descriptor_set(const DescriptorSet& descriptor_set) const
{
    m_impl->bind_descriptor_sets(1, { &descriptor_set, nullptr, nullptr, nullptr });
}

UniformBuffer Renderer::create_uniform_buffer(const ShaderDescriptorBinding& descriptor_binding)
{
    MVE_VAL_ASSERT(
        descriptor_binding.type() == ShaderDescriptorType::uniform_buffer,
        "[Renderer] Failed to create uniform buffer as binding is not of type uniform buffer")

    const uint32_t struct_size = descriptor_binding.block().size();

    const FrameInFlight& ref_frame = m_impl->frames_in_flight.at(0);
    std::optional<Handle> id;
    for (Handle i = 0; i < ref_frame.uniform_buffers.size(); i++) {
        if (!ref_frame.uniform_buffers[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = ref_frame.uniform_buffers.size();

        for (FrameInFlight& frame : m_impl->frames_in_flight) {
            frame.uniform_buffers.emplace_back();
        }
    }
    int i = 0;

    for (FrameInFlight& frame : m_impl->frames_in_flight) {
        const Buffer buffer = create_buffer(
            m_impl->vma_allocator,
            struct_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO,
            VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        void* ptr;
        vmaMapMemory(m_impl->vma_allocator, buffer.vma_allocation, &ptr);
        frame.uniform_buffers[*id] = { buffer, struct_size, static_cast<std::byte*>(ptr) };
        i++;
    }

    Logger::debug(std::format("[Renderer] Uniform buffer created with ID: {}", *id));

    return { *this, *id };
}

void Renderer::update_uniform(
    UniformBuffer& uniform_buffer, const UniformLocation location, const float value, const bool persist)
{
    m_impl->update_uniform<float>(uniform_buffer, location, value, persist);
}

void Renderer::update_uniform(
    UniformBuffer& uniform_buffer, const UniformLocation location, const nnm::Vector2f value, const bool persist)
{
    m_impl->update_uniform<nnm::Vector2f>(uniform_buffer, location, value, persist);
}

void Renderer::update_uniform(
    UniformBuffer& uniform_buffer, const UniformLocation location, const nnm::Vector3f value, const bool persist)
{
    m_impl->update_uniform<nnm::Vector3f>(uniform_buffer, location, value, persist);
}

void Renderer::update_uniform(
    UniformBuffer& uniform_buffer, const UniformLocation location, const nnm::Vector4f value, const bool persist)
{
    m_impl->update_uniform<nnm::Vector4f>(uniform_buffer, location, value, persist);
}

void Renderer::update_uniform(
    UniformBuffer& uniform_buffer, const UniformLocation location, const nnm::Matrix3f& value, const bool persist)
{
    m_impl->update_uniform<nnm::Matrix3f>(uniform_buffer, location, value, persist);
}

void Renderer::update_uniform(
    UniformBuffer& uniform_buffer, const UniformLocation location, const nnm::Matrix4f& value, const bool persist)
{
    m_impl->update_uniform<nnm::Matrix4f>(uniform_buffer, location, value, persist);
}

void Renderer::update_uniform(
    UniformBuffer& uniform_buffer, const UniformLocation location, const bool value, const bool persist)
{
    m_impl->update_uniform<uint32_t>(uniform_buffer, location, value, persist);
}

void Renderer::destroy(Texture& texture)
{
    MVE_VAL_ASSERT(texture.is_valid(), "[Renderer] Attempted to destroy invalid texture")
    Logger::debug(std::format("[Renderer] Destroyed texture with ID: {}", texture.handle()));
    const Handle handle = texture.handle();
    texture.invalidate();
    m_impl->deferred_operations.emplace_back(DeferredDestroyTexture { handle });
}

// TODO: mip-mapping
Texture Renderer::create_texture(
    const TextureFormat format, const uint32_t width, const uint32_t height, const std::byte* data)
{
    MVE_VAL_ASSERT(width != 0 && height != 0, "[Renderer] Attempt to create texture with 0 width or height")
    constexpr uint32_t mip_levels = 1;

    vk::Format vk_format {};
    size_t size = width * height * sizeof(std::byte);
    switch (format) {
    case TextureFormat::r:
        vk_format = vk::Format::eR8Unorm;
        break;
    case TextureFormat::rg:
        vk_format = vk::Format::eR8G8Unorm;
        size *= 2;
        break;
    case TextureFormat::rgb:
        vk_format = vk::Format::eR8G8B8Unorm;
        size *= 3;
        break;
    case TextureFormat::rgba:
        vk_format = vk::Format::eR8G8B8A8Unorm;
        size *= 4;
        break;
    }

    const Buffer staging_buffer = create_buffer(
        m_impl->vma_allocator,
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* data_ptr;
    vmaMapMemory(m_impl->vma_allocator, staging_buffer.vma_allocation, &data_ptr);
    memcpy(data_ptr, data, size);
    vmaUnmapMemory(m_impl->vma_allocator, staging_buffer.vma_allocation);

    const Image image = create_image(
        m_impl->vma_allocator,
        width,
        height,
        mip_levels,
        vk::SampleCountFlagBits::e1,
        vk_format,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        false);

    m_impl->deferred_operations.emplace_back(DeferredCopyBufferImage {
        .mip_levels = mip_levels, .staging_buffer = staging_buffer, .dst_image = image, .image_format = vk_format });

    const vk::ImageView image_view = create_image_view(
        m_impl->vk_loader, m_impl->vk_device, image.vk_handle, vk_format, vk::ImageAspectFlagBits::eColor, mip_levels);

    const vk::Sampler sampler
        = create_texture_sampler(m_impl->vk_loader, m_impl->vk_physical_device, m_impl->vk_device, mip_levels);

    Handle handle = m_impl->textures.allocate(
        TextureImpl { .image = image, .vk_image_view = image_view, .vk_sampler = sampler, .mip_levels = mip_levels });

    Logger::debug(std::format("[Renderer] Texture created with ID: {}", handle));

    return { *this, handle };
}

Texture Renderer::create_texture(const std::filesystem::path& path)
{
    int width;
    int height;
    int channels;
    const std::string path_string = path.string();
    const stbi_uc* pixels = stbi_load(path_string.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    // vk::DeviceSize size = width * height * 4;
    MVE_ASSERT(pixels != nullptr, "[Renderer] Failed to load texture image")

    return create_texture(
        TextureFormat::rgba,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        reinterpret_cast<const std::byte*>(pixels));
}

void Renderer::draw_vertex_buffer(const VertexBuffer& vertex_buffer)
{
    auto& [buffer, vertex_count] = m_impl->vertex_buffers.get(vertex_buffer.handle());
    m_impl->current_draw_state.command_buffer.bindVertexBuffers(0, buffer.vk_handle, { 0 }, m_impl->vk_loader);
    m_impl->current_draw_state.command_buffer.draw(vertex_count, 1, 0, 0, m_impl->vk_loader);
}

void Renderer::destroy(DescriptorSet& descriptor_set)
{
    MVE_VAL_ASSERT(descriptor_set.is_valid(), "[Renderer] Attempted to destroy invalid descriptor set")
    Logger::debug(std::format("[Renderer] Destroyed descriptor set with ID: {}", descriptor_set.handle()));
    const Handle handle = descriptor_set.handle();
    descriptor_set.invalidate();
    m_impl->deferred_operations.emplace_back(DeferredDestroyDescriptorSet { handle });
}

void Renderer::destroy(GraphicsPipeline& graphics_pipeline)
{
    MVE_VAL_ASSERT(graphics_pipeline.is_valid(), "[Renderer] Attempted to destroy invalid graphics pipeline")
    Logger::debug(std::format("[Renderer] Destroyed graphics pipeline with ID: {}", graphics_pipeline.handle()));
    const Handle handle = graphics_pipeline.handle();
    graphics_pipeline.invalidate();
    m_impl->deferred_operations.emplace_back(DeferredDestroyGraphicsPipeline { handle });
}

void Renderer::destroy(UniformBuffer& uniform_buffer)
{
    MVE_VAL_ASSERT(uniform_buffer.is_valid(), "[Renderer] Attempted to destroy invalid uniform buffer")
    Logger::debug(std::format("[Renderer] Destroyed uniform buffer with ID: {}", uniform_buffer.handle()));
    uniform_buffer.invalidate();
    const Handle handle = uniform_buffer.handle();
    m_impl->deferred_operations.emplace_back(DeferredDestroyUniformBuffer { handle });
}

void Renderer::destroy(IndexBuffer& index_buffer)
{
    MVE_VAL_ASSERT(index_buffer.is_valid(), "[Renderer] Attempted to destroy invalid index buffer")
    Logger::debug(std::format("[Renderer] Destroyed index buffer with ID: {}", index_buffer.handle()));
    const Handle handle = index_buffer.handle();
    index_buffer.invalidate();
    m_impl->deferred_operations.emplace_back(DeferredDestroyIndexBuffer { handle });
}

void Renderer::bind_descriptor_sets(const DescriptorSet& descriptor_set_1, const DescriptorSet& descriptor_set_2) const
{
    m_impl->bind_descriptor_sets(2, { &descriptor_set_1, &descriptor_set_2, nullptr, nullptr });
}

void Renderer::bind_descriptor_sets(
    const DescriptorSet& descriptor_set_1,
    const DescriptorSet& descriptor_set_2,
    const DescriptorSet& descriptor_set_3) const
{
    m_impl->bind_descriptor_sets(3, { &descriptor_set_1, &descriptor_set_2, &descriptor_set_3, nullptr });
}

void Renderer::bind_descriptor_sets(
    const DescriptorSet& descriptor_set_1,
    const DescriptorSet& descriptor_set_2,
    const DescriptorSet& descriptor_set_3,
    const DescriptorSet& descriptor_set_4) const
{
    m_impl->bind_descriptor_sets(4, { &descriptor_set_1, &descriptor_set_2, &descriptor_set_3, &descriptor_set_4 });
}

void Renderer::end_render_pass() const
{
    m_impl->current_draw_state.command_buffer.endRenderPass(m_impl->vk_loader);
}

Framebuffer Renderer::create_framebuffer()
{
    Handle handle = m_impl->framebuffers.allocate(std::move(m_impl->create_framebuffer_impl(*this, m_impl->vk_loader)));

    Logger::debug(std::format("[Renderer] Framebuffer created with ID: {}", handle));

    return { *this, handle };
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void Renderer::set_framebuffer_resize_callback(Framebuffer& framebuffer, std::function<void()> callback)
{
    MVE_VAL_ASSERT(framebuffer.is_valid(), "[Renderer] Attempt to set resize callback on invalid FrameBuffer");
    m_impl->framebuffers.get(framebuffer.handle()).callback = std::move(callback);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void Renderer::remove_framebuffer_resize_callback(Framebuffer& framebuffer)
{
    MVE_VAL_ASSERT(framebuffer.is_valid(), "[Renderer] Attempt to remove resize callback on invalid FrameBuffer");
    m_impl->framebuffers.get(framebuffer.handle()).callback.reset();
}

void Renderer::destroy(Framebuffer& framebuffer)
{
    MVE_VAL_ASSERT(framebuffer.is_valid(), "[Renderer] Attempted to destroy invalid framebuffer")
    Logger::debug(std::format("[Renderer] Destroyed framebuffer with ID: {}", framebuffer.handle()));
    const Handle handle = framebuffer.handle();
    framebuffer.invalidate();
    m_impl->deferred_operations.emplace_back(DeferredDestroyFramebuffer { handle });
}

const Texture& Renderer::framebuffer_texture(const Framebuffer& framebuffer)
{
    return m_impl->framebuffers.get(framebuffer.handle()).texture;
}

nnm::Vector2i Renderer::framebuffer_size(const Framebuffer& framebuffer) const
{
    return m_impl->framebuffers.get(framebuffer.handle()).size;
}

std::string Renderer::gpu_name() const
{
    return m_impl->vk_physical_device.getProperties(m_impl->vk_loader).deviceName;
}

nnm::Vector2i Renderer::texture_size(const Texture& texture) const
{
    MVE_VAL_ASSERT(texture.is_valid(), "[Renderer] Attempt to get size on invalid texture")
    const auto& [image, vk_image_view, vk_sampler, mip_levels] = m_impl->textures.get(texture.handle());
    return { static_cast<int>(image.width), static_cast<int>(image.height) };
}

Msaa Renderer::max_msaa_samples() const
{
    return m_impl->max_msaa_samples;
}

void Renderer::set_msaa_samples(const Window& window, const Msaa samples)
{
    const vk::Result wait_result = m_impl->vk_device.waitIdle(m_impl->vk_loader);
    MVE_ASSERT(wait_result == vk::Result::eSuccess, "[Renderer] Failed to wait idle for setting MSAA")
    vk::SampleCountFlagBits vk_samples {};
    switch (samples) {
    case Msaa::samples_1:
        vk_samples = vk::SampleCountFlagBits::e1;
        break;
    case Msaa::samples_2:
        vk_samples = vk::SampleCountFlagBits::e2;
        break;
    case Msaa::samples_4:
        vk_samples = vk::SampleCountFlagBits::e4;
        break;
    case Msaa::samples_8:
        vk_samples = vk::SampleCountFlagBits::e8;
        break;
    case Msaa::samples_16:
        vk_samples = vk::SampleCountFlagBits::e16;
        break;
    case Msaa::samples_32:
        vk_samples = vk::SampleCountFlagBits::e32;
        break;
    case Msaa::samples_64:
        vk_samples = vk::SampleCountFlagBits::e64;
        break;
    }
    m_impl->msaa_samples = vk_samples;
    m_impl->recreate_render_passes();
    m_impl->recreate_graphics_pipelines();
    m_impl->recreate_swapchain(*this, window);
}

Msaa Renderer::current_msaa_samples() const
{
    return vk_samples_to_msaa(m_impl->msaa_samples);
}

}