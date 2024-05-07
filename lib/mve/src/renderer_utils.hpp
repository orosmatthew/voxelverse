#pragma once

#include <set>

#include <mve/common.hpp>
#include <mve/detail/defs.hpp>
#include <mve/shader.hpp>

#include "logger.hpp"

namespace mve::detail {

inline vk::SampleCountFlagBits get_max_sample_count(
    const vk::DispatchLoaderDynamic& loader, const vk::PhysicalDevice physical_device)
{
    const vk::PhysicalDeviceProperties properties = physical_device.getProperties(loader);

    const vk::SampleCountFlags counts
        = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

    if (counts & vk::SampleCountFlagBits::e64) {
        return vk::SampleCountFlagBits::e64;
    }
    if (counts & vk::SampleCountFlagBits::e32) {
        return vk::SampleCountFlagBits::e32;
    }
    if (counts & vk::SampleCountFlagBits::e16) {
        return vk::SampleCountFlagBits::e16;
    }
    if (counts & vk::SampleCountFlagBits::e8) {
        return vk::SampleCountFlagBits::e8;
    }
    if (counts & vk::SampleCountFlagBits::e4) {
        return vk::SampleCountFlagBits::e4;
    }
    if (counts & vk::SampleCountFlagBits::e2) {
        return vk::SampleCountFlagBits::e2;
    }
    return vk::SampleCountFlagBits::e1;
}

inline bool has_stencil_component(const vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

inline void cmd_copy_buffer_to_image(
    const vk::DispatchLoaderDynamic& loader,
    const vk::CommandBuffer command_buffer,
    const vk::Buffer buffer,
    const vk::Image image,
    const uint32_t width,
    const uint32_t height)
{
    const auto region
        = vk::BufferImageCopy()
              .setBufferOffset(0)
              .setBufferRowLength(0)
              .setBufferImageHeight(0)
              .setImageSubresource(vk::ImageSubresourceLayers()
                                       .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                       .setMipLevel(0)
                                       .setBaseArrayLayer(0)
                                       .setLayerCount(1))
              .setImageOffset(vk::Offset3D(0, 0, 0))
              .setImageExtent(vk::Extent3D(width, height, 1));

    command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region, loader);
}

inline vk::Sampler create_texture_sampler(
    const vk::DispatchLoaderDynamic& loader,
    const vk::PhysicalDevice physical_device,
    const vk::Device device,
    const uint32_t mip_levels)
{
    const auto sampler_info
        = vk::SamplerCreateInfo()
              .setMagFilter(vk::Filter::eNearest)
              .setMinFilter(vk::Filter::eNearest)
              //.setMagFilter(vk::Filter::eLinear)
              //.setMinFilter(vk::Filter::eLinear)
              .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
              .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
              .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
              .setAnisotropyEnable(VK_FALSE) // Disable with VK_FALSE
              .setMaxAnisotropy(physical_device.getProperties(loader).limits.maxSamplerAnisotropy) // Disable with 1.0f
              .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
              .setUnnormalizedCoordinates(VK_FALSE)
              .setCompareEnable(VK_FALSE)
              .setCompareOp(vk::CompareOp::eAlways)
              .setMipmapMode(vk::SamplerMipmapMode::eNearest)
              .setMipLodBias(0.0f)
              .setMinLod(0.0f)
              .setMaxLod(static_cast<float>(mip_levels));

    const vk::ResultValue<vk::Sampler> sampler_result = device.createSampler(sampler_info, nullptr, loader);
    MVE_ASSERT(sampler_result.result == vk::Result::eSuccess, "[Renderer] Failed to create image sampler")
    return sampler_result.value;
}

inline void cmd_transition_image_layout(
    const vk::DispatchLoaderDynamic& loader,
    const vk::CommandBuffer command_buffer,
    const vk::Image image,
    const vk::Format format,
    const vk::ImageLayout old_layout,
    const vk::ImageLayout new_layout,
    const uint32_t mip_levels)
{
    auto barrier
        = vk::ImageMemoryBarrier()
              .setOldLayout(old_layout)
              .setNewLayout(new_layout)
              .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
              .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
              .setImage(image)
              .setSubresourceRange(vk::ImageSubresourceRange()
                                       .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                       .setBaseMipLevel(0)
                                       .setLevelCount(mip_levels)
                                       .setBaseArrayLayer(0)
                                       .setLayerCount(1));

    vk::PipelineStageFlags source_stage;
    vk::PipelineStageFlags destination_stage;

    if (new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);
        if (has_stencil_component(format)) {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    }

    if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        destination_stage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (
        old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {

        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        source_stage = vk::PipelineStageFlagBits::eTransfer;
        destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (
        old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask
            = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        destination_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else {
        MVE_ASSERT(false, "[Renderer] Unsupported layout transition")
    }

    command_buffer.pipelineBarrier(
        source_stage,
        destination_stage,
        vk::DependencyFlagBits::eByRegion,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier,
        loader);
}

inline vk::Format find_supported_format(
    const vk::DispatchLoaderDynamic& loader,
    const vk::PhysicalDevice physical_device,
    const std::vector<vk::Format>& formats,
    const vk::ImageTiling tiling,
    const vk::FormatFeatureFlags features)
{
    for (const vk::Format format : formats) {
        if (vk::FormatProperties properties = physical_device.getFormatProperties(format, loader);
            (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features)
            || (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)) {
            return format;
        }
    }
    MVE_ASSERT(false, "[Renderer] Failed to find supported format")
}

inline vk::Format find_depth_format(const vk::DispatchLoaderDynamic& loader, const vk::PhysicalDevice physical_device)
{
    const std::vector formats = { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };
    return find_supported_format(
        loader,
        physical_device,
        formats,
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

inline Image create_image(
    const VmaAllocator allocator,
    const uint32_t width,
    const uint32_t height,
    const uint32_t mip_levels,
    vk::SampleCountFlagBits samples,
    vk::Format format,
    vk::ImageTiling tiling,
    const vk::ImageUsageFlags usage,
    const bool dedicated)
{
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = mip_levels;
    image_info.arrayLayers = 1;
    image_info.format = static_cast<VkFormat>(format);
    image_info.tiling = static_cast<VkImageTiling>(tiling);
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = static_cast<VkImageUsageFlags>(usage);
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = static_cast<VkSampleCountFlagBits>(samples);

    VmaAllocationCreateInfo vma_alloc_info = {};
    vma_alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    if (dedicated) {
        vma_alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    }

    VkImage image;
    VmaAllocation image_allocation;
    vmaCreateImage(allocator, &image_info, &vma_alloc_info, &image, &image_allocation, nullptr);
    return { vk::Image(image), image_allocation, width, height };
}

inline vk::ImageView create_image_view(
    const vk::DispatchLoaderDynamic& loader,
    const vk::Device device,
    const vk::Image image,
    const vk::Format format,
    const vk::ImageAspectFlags aspect_flags,
    const uint32_t mip_levels)
{
    constexpr auto components
        = vk::ComponentMapping()
              .setR(vk::ComponentSwizzle::eIdentity)
              .setG(vk::ComponentSwizzle::eIdentity)
              .setB(vk::ComponentSwizzle::eIdentity)
              .setA(vk::ComponentSwizzle::eIdentity);

    const auto image_subresource_range
        = vk::ImageSubresourceRange()
              .setAspectMask(aspect_flags)
              .setBaseMipLevel(0)
              .setLevelCount(mip_levels)
              .setBaseArrayLayer(0)
              .setLayerCount(1);

    const auto view_info
        = vk::ImageViewCreateInfo()
              .setImage(image)
              .setViewType(vk::ImageViewType::e2D)
              .setFormat(format)
              .setComponents(components)
              .setSubresourceRange(image_subresource_range);

    const vk::ResultValue<vk::ImageView> image_view_result = device.createImageView(view_info, nullptr, loader);
    MVE_ASSERT(image_view_result.result == vk::Result::eSuccess, "[Renderer] Failed to create image view")
    return image_view_result.value;
}

inline vk::CommandBuffer begin_single_submit(
    const vk::DispatchLoaderDynamic& loader, const vk::Device device, const vk::CommandPool pool)
{
    const auto command_buffer_alloc_info
        = vk::CommandBufferAllocateInfo()
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandPool(pool)
              .setCommandBufferCount(1);

    const vk::ResultValue<std::vector<vk::CommandBuffer>> command_buffer_result
        = device.allocateCommandBuffers(command_buffer_alloc_info, loader);
    MVE_ASSERT(
        command_buffer_result.result == vk::Result::eSuccess, "[Renderer] Failed to allocate texture command buffer")
    const vk::CommandBuffer command_buffer = command_buffer_result.value.at(0);

    constexpr auto begin_info = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    const vk::Result begin_result = command_buffer.begin(begin_info, loader);
    MVE_ASSERT(begin_result == vk::Result::eSuccess, "[Renderer] Failed to begin single submit buffer")

    return command_buffer;
}

inline void end_single_submit(
    const vk::DispatchLoaderDynamic& loader,
    const vk::Device device,
    const vk::CommandPool pool,
    const vk::CommandBuffer command_buffer,
    const vk::Queue queue)
{
    const vk::Result end_result = command_buffer.end(loader);
    MVE_ASSERT(end_result == vk::Result::eSuccess, "[Renderer] Failed to end single submit buffer")

    const auto submit_info = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&command_buffer);

    const vk::Result submit_result = queue.submit(1, &submit_info, VK_NULL_HANDLE, loader);
    MVE_ASSERT(submit_result == vk::Result::eSuccess, "[Renderer] Failed to submit single submit buffer")
    const vk::Result wait_result = queue.waitIdle(loader);
    MVE_ASSERT(wait_result == vk::Result::eSuccess, "[Renderer] Failed to wait for queue for single submit")

    device.freeCommandBuffers(pool, 1, &command_buffer, loader);
}

inline RenderImage create_color_image(
    const vk::DispatchLoaderDynamic& loader,
    const vk::Device device,
    const VmaAllocator allocator,
    const vk::Extent2D swapchain_extent,
    const vk::Format swapchain_format,
    const vk::SampleCountFlagBits samples)
{
    const Image color_image = create_image(
        allocator,
        swapchain_extent.width,
        swapchain_extent.height,
        1,
        samples,
        swapchain_format,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        true); // TODO: make the sampled optional

    const vk::ImageView image_view = create_image_view(
        loader, device, color_image.vk_handle, swapchain_format, vk::ImageAspectFlagBits::eColor, 1);

    return { color_image, image_view };
}

inline void cmd_generate_mipmaps(
    const vk::DispatchLoaderDynamic& loader,
    [[maybe_unused]] const vk::PhysicalDevice physical_device,
    const vk::CommandBuffer command_buffer,
    const vk::Image image,
    [[maybe_unused]] const vk::Format format,
    const uint32_t width,
    const uint32_t height,
    const uint32_t mip_levels)
{
    // Check image format supports linear blitting
#ifdef MVE_ENABLE_VALIDATION
    const vk::FormatProperties properties = physical_device.getFormatProperties(format, loader);
    MVE_VAL_ASSERT(
        properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear,
        "[Renderer] Image format does not support linear blitting for mip-mapping")
#endif

    constexpr auto subresource_range
        = vk::ImageSubresourceRange()
              .setAspectMask(vk::ImageAspectFlagBits::eColor)
              .setBaseArrayLayer(0)
              .setLayerCount(1)
              .setLevelCount(1);

    auto barrier = vk::ImageMemoryBarrier()
                       .setImage(image)
                       .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                       .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                       .setSubresourceRange(subresource_range);

    uint32_t mip_width = width;
    uint32_t mip_height = height;

    for (uint32_t i = 1; i < mip_levels; i++) {
        barrier.subresourceRange.setBaseMipLevel(i - 1);
        barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
        barrier.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            {},
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);

        std::array src_offsets = { vk::Offset3D(0, 0, 0),
                                   vk::Offset3D(static_cast<int32_t>(mip_width), static_cast<int32_t>(mip_height), 1) };

        auto src_subresource
            = vk::ImageSubresourceLayers()
                  .setAspectMask(vk::ImageAspectFlagBits::eColor)
                  .setMipLevel(i - 1)
                  .setBaseArrayLayer(0)
                  .setLayerCount(1);

        std::array dst_offsets
            = { vk::Offset3D(0, 0, 0),
                vk::Offset3D(
                    static_cast<int32_t>(mip_width > 1 ? mip_width / 2 : 1),
                    static_cast<int32_t>(mip_height > 1 ? mip_height / 2 : 1),
                    1) };

        auto dst_subresource
            = vk::ImageSubresourceLayers()
                  .setAspectMask(vk::ImageAspectFlagBits::eColor)
                  .setMipLevel(i)
                  .setBaseArrayLayer(0)
                  .setLayerCount(1);

        auto blit = vk::ImageBlit()
                        .setSrcOffsets(src_offsets)
                        .setSrcSubresource(src_subresource)
                        .setDstOffsets(dst_offsets)
                        .setDstSubresource(dst_subresource);

        command_buffer.blitImage(
            image,
            vk::ImageLayout::eTransferSrcOptimal,
            image,
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &blit,
            vk::Filter::eNearest);

        barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
        barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            {},
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);

        if (mip_width > 1) {
            mip_width /= 2;
        }
        if (mip_height > 1) {
            mip_height /= 2;
        }
    }

    // Transition last mip because loop did not read from and transition it
    barrier.subresourceRange.setBaseMipLevel(mip_levels - 1);
    barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
    barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
    barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

    command_buffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        {},
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier,
        loader);
}

inline VkBool32 vk_debug_callback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    [[maybe_unused]] void* user_data)
{
    if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        log().warn("[Vulkan Debug] " + std::string(callback_data->pMessage));
    }

    return false;
}

// ReSharper disable once CppDFAUnreachableFunctionCall
inline std::vector<const char*> get_vk_validation_layer_exts()
{
    return std::vector { "VK_LAYER_KHRONOS_validation" };
}

// ReSharper disable once CppDFAUnreachableFunctionCall
inline bool has_validation_layer_support(const vk::DispatchLoaderDynamic& loader)
{
    vk::ResultValue<std::vector<vk::LayerProperties>> available_layers_result
        = enumerateInstanceLayerProperties(loader);
    MVE_VAL_ASSERT(
        available_layers_result.result == vk::Result::eSuccess, "[Renderer] Failed to get validation layer properties")

    const std::vector<vk::LayerProperties> available_layers = std::move(available_layers_result.value);

    for (const std::vector<const char*> validation_layers = get_vk_validation_layer_exts();
         const std::string& validation_layer : validation_layers) {
        if (!std::ranges::any_of(available_layers, [&](const vk::LayerProperties& available_layer) {
                if (std::strlen(available_layer.layerName.data()) != validation_layer.size()) {
                    return false;
                }
                for (int i = 0; i < validation_layer.size(); i++) {
                    if (available_layer.layerName.at(i) != validation_layer.at(i)) {
                        return false;
                    }
                }
                return true;
            })) {
            return false;
        }
    }
    return true;
}

inline std::vector<const char*> get_vk_instance_required_exts()
{
    uint32_t glfw_ext_count = 0;
    const char** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

    std::vector exts(glfw_exts, glfw_exts + glfw_ext_count);

    exts.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifdef MVE_ENABLE_VALIDATION
    exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return exts;
}

inline vk::Instance create_vk_instance(
    const std::string& app_name, const int app_version_major, const int app_version_minor, const int app_version_patch)
{
    const vk::DispatchLoaderDynamic temp_loader(vkGetInstanceProcAddr);
    MVE_VAL_ASSERT(
        has_validation_layer_support(temp_loader), "[Renderer] Validation layers requested but not available")

    const auto application_info
        = vk::ApplicationInfo()
              .setPApplicationName(app_name.c_str())
              .setApplicationVersion(VK_MAKE_VERSION(app_version_major, app_version_minor, app_version_patch))
              .setPEngineName("Mini Vulkan Engine")
              .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
              .setApiVersion(VK_API_VERSION_1_1);

    const std::vector<const char*> exts = get_vk_instance_required_exts();

#ifdef MVE_ENABLE_VALIDATION
    const std::vector<const char*> validation_layers = get_vk_validation_layer_exts();

    auto instance_create_info
        = vk::InstanceCreateInfo()
              .setPApplicationInfo(&application_info)
              .setEnabledExtensionCount(exts.size())
              .setPpEnabledExtensionNames(exts.data())
              .setEnabledLayerCount(validation_layers.size())
              .setPpEnabledLayerNames(validation_layers.data());

#else
    auto instance_create_info
        = vk::InstanceCreateInfo()
              .setPApplicationInfo(&application_info)
              .setEnabledExtensionCount(exts.size())
              .setPpEnabledExtensionNames(exts.data())
              .setEnabledLayerCount(0);
#endif
    const vk::ResultValue<vk::Instance> instance_result = createInstance(instance_create_info, nullptr, temp_loader);
    MVE_ASSERT(instance_result.result == vk::Result::eSuccess, "[Renderer] Failed to create instance")
    return instance_result.value;
}

inline vk::DebugUtilsMessengerEXT create_vk_debug_messenger(const vk::Instance instance)
{
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = vk_debug_callback;
    debug_create_info.pUserData = nullptr;

    const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    MVE_VAL_ASSERT(func != nullptr, "[Renderer] Failed to create Vulkan debug messenger")

    VkDebugUtilsMessengerEXT debug_messenger;
    func(instance, &debug_create_info, nullptr, &debug_messenger);
    return { debug_messenger };
}

inline std::vector<const char*> get_vk_device_required_exts()
{
    return std::vector { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}

inline QueueFamilyIndices get_vk_queue_family_indices(
    const vk::DispatchLoaderDynamic& loader, const vk::PhysicalDevice physical_device, const vk::SurfaceKHR surface)
{
    QueueFamilyIndices indices;

    const std::vector<vk::QueueFamilyProperties> queue_families = physical_device.getQueueFamilyProperties(loader);
    int i = 0;
    for (const vk::QueueFamilyProperties& queue_family : queue_families) {
        if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics_family = i;
        }

        const vk::ResultValue<unsigned int> surface_support_result
            = physical_device.getSurfaceSupportKHR(i, surface, loader);
        MVE_ASSERT(
            surface_support_result.result == vk::Result::eSuccess, "[Renderer] Failed to get GPU surface support")

        if (surface_support_result.value) {
            indices.present_family = i;
        }

        if (indices.is_complete()) {
            break;
        }

        i++;
    }
    return indices;
}

inline SwapchainSupportDetails get_vk_swapchain_support_details(
    const vk::DispatchLoaderDynamic& loader, const vk::PhysicalDevice physical_device, const vk::SurfaceKHR surface)
{
    SwapchainSupportDetails details;

    const vk::ResultValue<vk::SurfaceCapabilitiesKHR> capabilities_result
        = physical_device.getSurfaceCapabilitiesKHR(surface, loader);
    MVE_ASSERT(capabilities_result.result == vk::Result::eSuccess, "[Renderer] Failed to get surface capabilities")
    details.capabilities = capabilities_result.value;

    vk::ResultValue<std::vector<vk::SurfaceFormatKHR>> formats_result
        = physical_device.getSurfaceFormatsKHR(surface, loader);
    MVE_ASSERT(formats_result.result == vk::Result::eSuccess, "[Renderer] Failed to get surface formats")
    details.formats = std::move(formats_result.value);

    vk::ResultValue<std::vector<vk::PresentModeKHR>> present_modes_result
        = physical_device.getSurfacePresentModesKHR(surface, loader);
    MVE_ASSERT(present_modes_result.result == vk::Result::eSuccess, "[Renderer] Failed to get surface present modes")
    details.present_modes = std::move(present_modes_result.value);

    return details;
}

inline bool is_vk_physical_device_suitable(
    const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
    QueueFamilyIndices indices = get_vk_queue_family_indices(loader, physical_device, surface);

    vk::ResultValue<std::vector<vk::ExtensionProperties>> available_exts_result
        = physical_device.enumerateDeviceExtensionProperties(nullptr, loader);
    MVE_ASSERT(
        available_exts_result.result == vk::Result::eSuccess, "[Renderer] Failed to get device extension properties")

    std::vector<vk::ExtensionProperties> available_exts = std::move(available_exts_result.value);

    for (std::vector<const char*> required_exts = get_vk_device_required_exts();
         const std::string& required_ext : required_exts) {
        if (!std::ranges::any_of(std::as_const(available_exts), [&](const vk::ExtensionProperties& ext_props) {
                return required_ext == ext_props.extensionName;
            })) {
            return false;
        }
    }

    auto [capabilities, formats, present_modes] = get_vk_swapchain_support_details(loader, physical_device, surface);
    bool is_swapchain_adequate = !formats.empty() && !present_modes.empty();

    vk::PhysicalDeviceFeatures supported_features = physical_device.getFeatures(loader);

    return indices.is_complete() && is_swapchain_adequate && supported_features.samplerAnisotropy;
}

inline vk::PhysicalDevice pick_vk_physical_device(
    const vk::Instance instance, const vk::DispatchLoaderDynamic& loader, const vk::SurfaceKHR surface)
{
    vk::ResultValue<std::vector<vk::PhysicalDevice>> physical_devices_result
        = instance.enumeratePhysicalDevices(loader);
    MVE_ASSERT(
        physical_devices_result.result == vk::Result::eSuccess, "[Renderer] Failed to get physical devices (GPUs)")

    const std::vector<vk::PhysicalDevice> physical_devices = std::move(physical_devices_result.value);
    MVE_ASSERT(!physical_devices.empty(), "[Renderer] Failed to find Vulkan device")

    for (vk::PhysicalDevice physical_device : physical_devices) {
        log().debug("[Renderer] GPU Found: {0}", physical_device.getProperties(loader).deviceName);
    }

    for (const vk::PhysicalDevice physical_device : physical_devices) {
        if (is_vk_physical_device_suitable(loader, physical_device, surface)) {
            log().info("[Renderer] Using GPU: {0}", physical_device.getProperties(loader).deviceName);
            return physical_device;
        }
    }

    MVE_ASSERT(false, "[Renderer] Failed to find a suitable Vulkan device")
}

inline vk::Device create_vk_logical_device(
    const vk::DispatchLoaderDynamic& loader,
    vk::PhysicalDevice physical_device,
    QueueFamilyIndices queue_family_indices)
{
    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

    assert(queue_family_indices.is_complete());

    std::set _queue_families
        = { queue_family_indices.graphics_family.value(), queue_family_indices.present_family.value() };

    float queue_priority = 1.0f;

    for (uint32_t queue_family : _queue_families) {
        auto queue_create_info
            = vk::DeviceQueueCreateInfo()
                  .setQueueFamilyIndex(queue_family)
                  .setQueueCount(1)
                  .setPQueuePriorities(&queue_priority);

        queue_create_infos.push_back(queue_create_info);
    }

    vk::PhysicalDeviceFeatures device_features;
    device_features.samplerAnisotropy = VK_TRUE;
    device_features.sampleRateShading = VK_TRUE;

    std::vector<const char*> required_exts = get_vk_device_required_exts();

    required_exts.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

#ifdef MVE_ENABLE_VALIDATION
    const std::vector<const char*> validation_layers = get_vk_validation_layer_exts();

    auto device_create_info
        = vk::DeviceCreateInfo()
              .setPQueueCreateInfos(queue_create_infos.data())
              .setQueueCreateInfoCount(queue_create_infos.size())
              .setPEnabledFeatures(&device_features)
              .setEnabledExtensionCount(required_exts.size())
              .setPpEnabledExtensionNames(required_exts.data())
              .setEnabledLayerCount(validation_layers.size())
              .setPpEnabledLayerNames(validation_layers.data());
#else
    auto device_create_info
        = vk::DeviceCreateInfo()
              .setPQueueCreateInfos(queue_create_infos.data())
              .setQueueCreateInfoCount(queue_create_infos.size())
              .setPEnabledFeatures(&device_features)
              .setEnabledExtensionCount(required_exts.size())
              .setPpEnabledExtensionNames(required_exts.data())
              .setEnabledLayerCount(0);
#endif

    vk::ResultValue<vk::Device> device_result = physical_device.createDevice(device_create_info, nullptr, loader);
    MVE_ASSERT(device_result.result == vk::Result::eSuccess, "[Renderer] Failed to create device")
    return device_result.value;
}

inline vk::SurfaceKHR create_vk_surface(const vk::Instance instance, GLFWwindow* window)
{
    VkSurfaceKHR surface;
    const VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    MVE_ASSERT(result == VK_SUCCESS, "[Renderer] Failed to create window surface")
    return { surface };
}

inline vk::SurfaceFormatKHR choose_vk_swapchain_surface_format(
    const std::vector<vk::SurfaceFormatKHR>& available_formats)
{
    if (const auto it = std::ranges::find_if(
            available_formats,
            [](const vk::SurfaceFormatKHR& available_format) {
                return available_format.format == vk::Format::eB8G8R8A8Unorm
                    && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
            });
        it != available_formats.end()) {
        return *it;
    }
    return available_formats[0];
}

inline vk::PresentModeKHR choose_vk_swapchain_present_mode(
    const std::vector<vk::PresentModeKHR>& available_present_modes)
{
    if (const auto it = std::ranges::find_if(
            available_present_modes,
            [](const vk::PresentModeKHR& available_present_mode) {
                return available_present_mode == vk::PresentModeKHR::eMailbox;
            });
        it != available_present_modes.end()) {
        return *it;
    }
    return vk::PresentModeKHR::eFifo;
}

inline vk::Extent2D get_vk_swapchain_extent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);

    vk::Extent2D actual_extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    actual_extent.width
        = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height
        = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actual_extent;
}

inline vk::SwapchainKHR create_vk_swapchain(
    const vk::DispatchLoaderDynamic& loader,
    const vk::PhysicalDevice physical_device,
    const vk::Device device,
    const vk::SurfaceKHR surface,
    const vk::SurfaceFormatKHR surface_format,
    const vk::Extent2D surface_extent,
    const QueueFamilyIndices queue_family_indices)
{
    auto [capabilities, formats, present_modes] = get_vk_swapchain_support_details(loader, physical_device, surface);

    const vk::PresentModeKHR present_mode = choose_vk_swapchain_present_mode(present_modes);

    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

    assert(queue_family_indices.is_complete());

    auto swapchain_create_info
        = vk::SwapchainCreateInfoKHR()
              .setSurface(surface)
              .setMinImageCount(image_count)
              .setImageFormat(surface_format.format)
              .setImageColorSpace(surface_format.colorSpace)
              .setImageExtent(surface_extent)
              .setImageArrayLayers(1)
              .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
              .setPreTransform(capabilities.currentTransform)
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setPresentMode(present_mode)
              .setClipped(true)
              .setOldSwapchain(nullptr);

    if (queue_family_indices.graphics_family != queue_family_indices.present_family) {
        const uint32_t indices_arr[]
            = { queue_family_indices.graphics_family.value(), queue_family_indices.present_family.value() };
        swapchain_create_info.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndexCount(2)
            .setPQueueFamilyIndices(indices_arr);
    }
    else {
        swapchain_create_info.setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount(0)
            .setPQueueFamilyIndices(nullptr);
    }

    const vk::ResultValue<vk::SwapchainKHR> swapchain_result
        = device.createSwapchainKHR(swapchain_create_info, nullptr, loader);
    MVE_ASSERT(swapchain_result.result == vk::Result::eSuccess, "[Renderer] Failed to create swapchain")
    return swapchain_result.value;
}

inline std::vector<vk::Image> get_vk_swapchain_images(
    const vk::DispatchLoaderDynamic& loader, const vk::Device device, const vk::SwapchainKHR swapchain)
{
    vk::ResultValue<std::vector<vk::Image>> images_result = device.getSwapchainImagesKHR(swapchain, loader);
    MVE_ASSERT(images_result.result == vk::Result::eSuccess, "[Renderer] Failed to get swapchain images")
    return images_result.value;
}

inline std::vector<vk::ImageView> create_vk_swapchain_image_views(
    const vk::DispatchLoaderDynamic& loader,
    const vk::Device device,
    const std::vector<vk::Image>& swapchain_images,
    const vk::Format image_format)
{
    std::vector<vk::ImageView> image_views;

    std::ranges::transform(swapchain_images, std::back_inserter(image_views), [&](const vk::Image swapchain_image) {
        return create_image_view(loader, device, swapchain_image, image_format, vk::ImageAspectFlagBits::eColor, 1);
    });

    return image_views;
}

inline vk::VertexInputBindingDescription create_vk_binding_description(const VertexLayout& layout)
{
    const auto binding_description
        = vk::VertexInputBindingDescription()
              .setBinding(0)
              .setStride(get_vertex_layout_bytes(layout))
              .setInputRate(vk::VertexInputRate::eVertex);
    return binding_description;
}

inline std::vector<vk::VertexInputAttributeDescription> create_vk_attribute_descriptions(const VertexLayout& layout)
{
    auto attribute_descriptions = std::vector<vk::VertexInputAttributeDescription>();
    attribute_descriptions.reserve(layout.size());

    int offset = 0;
    for (int i = 0; i < layout.size(); i++) {
        auto description = vk::VertexInputAttributeDescription().setBinding(0).setLocation(i).setOffset(offset);

        switch (layout[i]) {
        case VertexAttributeType::scalar:
            description.setFormat(vk::Format::eR32Sfloat);
            offset += sizeof(float);
            break;
        case VertexAttributeType::vec2:
            description.setFormat(vk::Format::eR32G32Sfloat);
            offset += sizeof(Vector2f);
            break;
        case VertexAttributeType::vec3:
            description.setFormat(vk::Format::eR32G32B32Sfloat);
            offset += sizeof(Vector3f);
            break;
        case VertexAttributeType::vec4:
            description.setFormat(vk::Format::eR32G32B32A32Sfloat);
            offset += sizeof(Vector4f);
            break;
        }

        attribute_descriptions.push_back(description);
    }

    return attribute_descriptions;
}

inline vk::Pipeline create_vk_graphics_pipeline(
    const vk::DispatchLoaderDynamic& loader,
    vk::Device device,
    const Shader& vertex_shader,
    const Shader& fragment_shader,
    vk::PipelineLayout pipeline_layout,
    vk::RenderPass render_pass,
    const VertexLayout& vertex_layout,
    vk::SampleCountFlagBits samples,
    bool depth_test)
{
    std::vector<uint32_t> vertex_spv_code = vertex_shader.spv_code();
    auto vertex_shader_create_info
        = vk::ShaderModuleCreateInfo().setCodeSize(vertex_spv_code.size() * 4).setPCode(vertex_spv_code.data());

    vk::ResultValue<vk::ShaderModule> vertex_shader_module_result
        = device.createShaderModule(vertex_shader_create_info, nullptr, loader);
    MVE_ASSERT(
        vertex_shader_module_result.result == vk::Result::eSuccess, "[Renderer] Failed to create vertex shader module")
    vk::ShaderModule vertex_shader_module = vertex_shader_module_result.value;

    std::vector<uint32_t> fragment_spv_code = fragment_shader.spv_code();
    auto fragment_shader_create_info
        = vk::ShaderModuleCreateInfo().setCodeSize(fragment_spv_code.size() * 4).setPCode(fragment_spv_code.data());

    vk::ResultValue<vk::ShaderModule> fragment_shader_module_result
        = device.createShaderModule(fragment_shader_create_info, nullptr, loader);
    MVE_ASSERT(
        fragment_shader_module_result.result == vk::Result::eSuccess,
        "[Renderer] Failed to create fragment shader module")
    vk::ShaderModule fragment_shader_module = fragment_shader_module_result.value;

    auto vertex_shader_stage_info
        = vk::PipelineShaderStageCreateInfo()
              .setStage(vk::ShaderStageFlagBits::eVertex)
              .setModule(vertex_shader_module)
              .setPName("main");

    auto fragment_shader_stage_info
        = vk::PipelineShaderStageCreateInfo()
              .setStage(vk::ShaderStageFlagBits::eFragment)
              .setModule(fragment_shader_module)
              .setPName("main");

    vk::PipelineShaderStageCreateInfo shader_stages[] = { vertex_shader_stage_info, fragment_shader_stage_info };

    vk::VertexInputBindingDescription binding_description = create_vk_binding_description(vertex_layout);
    std::vector<vk::VertexInputAttributeDescription> attribute_descriptions
        = create_vk_attribute_descriptions(vertex_layout);

    auto vertex_input_info
        = vk::PipelineVertexInputStateCreateInfo()
              .setVertexBindingDescriptionCount(1)
              .setPVertexBindingDescriptions(&binding_description)
              .setVertexAttributeDescriptionCount(attribute_descriptions.size())
              .setPVertexAttributeDescriptions(attribute_descriptions.data());

    auto input_assembly_info
        = vk::PipelineInputAssemblyStateCreateInfo()
              .setTopology(vk::PrimitiveTopology::eTriangleList)
              .setPrimitiveRestartEnable(false);

    std::vector dynamic_states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

    auto dynamic_state_info = vk::PipelineDynamicStateCreateInfo()
                                  .setDynamicStateCount(dynamic_states.size())
                                  .setPDynamicStates(dynamic_states.data());

    auto viewport_state = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setScissorCount(1);

    auto rasterizer
        = vk::PipelineRasterizationStateCreateInfo()
              .setDepthClampEnable(false)
              .setRasterizerDiscardEnable(false)
              .setPolygonMode(vk::PolygonMode::eFill)
              .setLineWidth(1.0f)
              .setCullMode(vk::CullModeFlagBits::eBack)
              .setFrontFace(vk::FrontFace::eCounterClockwise)
              .setDepthBiasEnable(false)
              .setDepthBiasConstantFactor(0.0f)
              .setDepthBiasClamp(0.0f)
              .setDepthBiasSlopeFactor(0.0f);

    auto multisampling_info
        = vk::PipelineMultisampleStateCreateInfo()
              .setSampleShadingEnable(VK_TRUE)
              .setMinSampleShading(1.0f) // Closer to 1.0f is smoother
              .setRasterizationSamples(samples)
              .setPSampleMask(nullptr)
              .setAlphaToCoverageEnable(false)
              .setAlphaToOneEnable(false);

    auto depth_stencil
        = vk::PipelineDepthStencilStateCreateInfo()
              .setDepthTestEnable(depth_test)
              .setDepthWriteEnable(VK_TRUE)
              .setDepthCompareOp(vk::CompareOp::eLess)
              .setDepthBoundsTestEnable(VK_FALSE)
              .setStencilTestEnable(VK_FALSE);

    auto color_blend_attachment
        = vk::PipelineColorBlendAttachmentState()
              .setColorWriteMask(
                  vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB
                  | vk::ColorComponentFlagBits::eA)
              .setBlendEnable(true)
              .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
              .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
              .setColorBlendOp(vk::BlendOp::eAdd)
              .setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
              .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
              .setAlphaBlendOp(vk::BlendOp::eAdd);

    auto color_blending_info
        = vk::PipelineColorBlendStateCreateInfo()
              .setLogicOpEnable(false)
              .setLogicOp(vk::LogicOp::eCopy)
              .setAttachmentCount(1)
              .setPAttachments(&color_blend_attachment)
              .setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

    auto graphics_pipeline_info
        = vk::GraphicsPipelineCreateInfo()
              .setStageCount(2)
              .setPStages(shader_stages)
              .setPVertexInputState(&vertex_input_info)
              .setPInputAssemblyState(&input_assembly_info)
              .setPViewportState(&viewport_state)
              .setPRasterizationState(&rasterizer)
              .setPMultisampleState(&multisampling_info)
              .setPDepthStencilState(&depth_stencil)
              .setPColorBlendState(&color_blending_info)
              .setPDynamicState(&dynamic_state_info)
              .setLayout(pipeline_layout)
              .setRenderPass(render_pass)
              .setSubpass(0)
              .setBasePipelineHandle(nullptr)
              .setBasePipelineIndex(-1);

    vk::ResultValue<vk::Pipeline> pipeline_result
        = device.createGraphicsPipeline(nullptr, graphics_pipeline_info, nullptr, loader);
    MVE_ASSERT(pipeline_result.result == vk::Result::eSuccess, "[Renderer] Failed to create graphics pipeline")
    vk::Pipeline graphics_pipeline = pipeline_result.value;

    device.destroy(vertex_shader_module, nullptr, loader);
    device.destroy(fragment_shader_module, nullptr, loader);

    return graphics_pipeline;
}

inline vk::RenderPass create_vk_render_pass(
    const vk::DispatchLoaderDynamic& loader,
    vk::Device device,
    vk::Format swapchain_format,
    vk::Format depth_format,
    vk::SampleCountFlagBits samples)
{
    auto color_attachment
        = vk::AttachmentDescription()
              .setFormat(swapchain_format)
              .setSamples(samples)
              .setLoadOp(vk::AttachmentLoadOp::eClear)
              .setStoreOp(vk::AttachmentStoreOp::eStore)
              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
              .setInitialLayout(vk::ImageLayout::eUndefined)
              .setFinalLayout(
                  samples == vk::SampleCountFlagBits::e1 ? vk::ImageLayout::ePresentSrcKHR
                                                         : vk::ImageLayout::eColorAttachmentOptimal);

    auto depth_attachment
        = vk::AttachmentDescription()
              .setFormat(depth_format)
              .setSamples(samples)
              .setLoadOp(vk::AttachmentLoadOp::eClear)
              .setStoreOp(vk::AttachmentStoreOp::eDontCare)
              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
              .setInitialLayout(vk::ImageLayout::eUndefined)
              .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::optional<vk::AttachmentDescription> color_attachment_resolve {};
    if (samples != vk::SampleCountFlagBits::e1) {
        color_attachment_resolve
            = vk::AttachmentDescription()
                  .setFormat(swapchain_format)
                  .setSamples(vk::SampleCountFlagBits::e1)
                  .setLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStoreOp(vk::AttachmentStoreOp::eStore)
                  .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .setInitialLayout(vk::ImageLayout::eUndefined)
                  .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    }

    auto color_attachment_ref
        = vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    auto depth_attachment_ref
        = vk::AttachmentReference().setAttachment(1).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::optional<vk::AttachmentReference> color_attachment_resolve_ref {};
    if (samples != vk::SampleCountFlagBits::e1) {
        color_attachment_resolve_ref
            = vk::AttachmentReference().setAttachment(2).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    }

    std::array<vk::AttachmentDescription, 3> attachments = { color_attachment, depth_attachment };
    if (color_attachment_resolve.has_value()) {
        attachments[2] = color_attachment_resolve.value();
    }

    auto subpass = vk::SubpassDescription()
                       .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                       .setColorAttachmentCount(1)
                       .setPColorAttachments(&color_attachment_ref)
                       .setPDepthStencilAttachment(&depth_attachment_ref);

    if (color_attachment_resolve_ref.has_value()) {
        subpass.setPResolveAttachments(&color_attachment_resolve_ref.value());
    }

    auto subpass_dependency
        = vk::SubpassDependency()
              .setSrcSubpass(VK_SUBPASS_EXTERNAL)
              .setDstSubpass(0)
              .setSrcStageMask(
                  vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests
                  | vk::PipelineStageFlagBits::eLateFragmentTests)
              .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
              .setDstStageMask(
                  vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests
                  | vk::PipelineStageFlagBits::eLateFragmentTests)
              .setDstAccessMask(
                  vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    auto render_pass_info
        = vk::RenderPassCreateInfo()
              .setAttachmentCount(samples == vk::SampleCountFlagBits::e1 ? 2 : 3)
              .setPAttachments(attachments.data())
              .setSubpassCount(1)
              .setPSubpasses(&subpass)
              .setSubpassCount(1)
              .setPDependencies(&subpass_dependency);

    vk::ResultValue<vk::RenderPass> render_pass_result = device.createRenderPass(render_pass_info, nullptr, loader);
    MVE_ASSERT(render_pass_result.result == vk::Result::eSuccess, "[Renderer] Failed to create render pass")
    return render_pass_result.value;
}

inline vk::RenderPass create_vk_render_pass_framebuffer(
    const vk::DispatchLoaderDynamic& loader,
    vk::Device device,
    vk::Format swapchain_format,
    vk::Format depth_format,
    vk::SampleCountFlagBits samples)
{
    auto color_attachment
        = vk::AttachmentDescription()
              .setFormat(swapchain_format)
              .setSamples(samples)
              .setLoadOp(vk::AttachmentLoadOp::eClear)
              .setStoreOp(vk::AttachmentStoreOp::eStore)
              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
              .setInitialLayout(vk::ImageLayout::eUndefined)
              .setFinalLayout(
                  samples == vk::SampleCountFlagBits::e1 ? vk::ImageLayout::eShaderReadOnlyOptimal
                                                         : vk::ImageLayout::eColorAttachmentOptimal);

    auto depth_attachment
        = vk::AttachmentDescription()
              .setFormat(depth_format)
              .setSamples(samples)
              .setLoadOp(vk::AttachmentLoadOp::eClear)
              .setStoreOp(vk::AttachmentStoreOp::eDontCare)
              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
              .setInitialLayout(vk::ImageLayout::eUndefined)
              .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::optional<vk::AttachmentDescription> color_attachment_resolve {};
    if (samples != vk::SampleCountFlagBits::e1) {
        color_attachment_resolve
            = vk::AttachmentDescription()
                  .setFormat(swapchain_format)
                  .setSamples(vk::SampleCountFlagBits::e1)
                  .setLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStoreOp(vk::AttachmentStoreOp::eStore)
                  .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .setInitialLayout(vk::ImageLayout::eUndefined)
                  .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    }

    auto color_attachment_ref
        = vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    auto depth_attachment_ref
        = vk::AttachmentReference().setAttachment(1).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::optional<vk::AttachmentReference> color_attachment_resolve_ref {};
    if (samples != vk::SampleCountFlagBits::e1) {
        color_attachment_resolve_ref
            = vk::AttachmentReference().setAttachment(2).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    }

    auto subpass = vk::SubpassDescription()
                       .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                       .setColorAttachmentCount(1)
                       .setPColorAttachments(&color_attachment_ref)
                       .setPDepthStencilAttachment(&depth_attachment_ref);

    if (color_attachment_resolve_ref.has_value()) {
        subpass.setPResolveAttachments(&color_attachment_resolve_ref.value());
    }

    std::array<vk::AttachmentDescription, 3> attachments = { color_attachment, depth_attachment };
    if (color_attachment_resolve.has_value()) {
        attachments[2] = color_attachment_resolve.value();
    }

    auto subpass_dependency
        = vk::SubpassDependency()
              .setSrcSubpass(VK_SUBPASS_EXTERNAL)
              .setDstSubpass(0)
              .setSrcStageMask(
                  vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests
                  | vk::PipelineStageFlagBits::eLateFragmentTests)
              .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
              .setDstStageMask(
                  vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests
                  | vk::PipelineStageFlagBits::eLateFragmentTests)
              .setDstAccessMask(
                  vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    auto render_pass_info
        = vk::RenderPassCreateInfo()
              .setAttachmentCount(samples == vk::SampleCountFlagBits::e1 ? 2 : 3)
              .setPAttachments(attachments.data())
              .setSubpassCount(1)
              .setPSubpasses(&subpass)
              .setSubpassCount(1)
              .setPDependencies(&subpass_dependency);

    vk::ResultValue<vk::RenderPass> render_pass_result = device.createRenderPass(render_pass_info, nullptr, loader);
    MVE_ASSERT(render_pass_result.result == vk::Result::eSuccess, "[Renderer] Failed to create render pass")
    return render_pass_result.value;
}

inline std::vector<vk::Framebuffer> create_vk_framebuffers(
    const vk::DispatchLoaderDynamic& loader,
    const vk::Device device,
    const std::vector<vk::ImageView>& swapchain_image_views,
    const vk::RenderPass render_pass,
    const vk::Extent2D swapchain_extent,
    const vk::ImageView color_image_view,
    const vk::ImageView depth_image_view,
    const vk::SampleCountFlagBits samples)
{
    std::vector<vk::Framebuffer> framebuffers;

    for (const vk::ImageView& swapchain_image_view : swapchain_image_views) {

        std::array<vk::ImageView, 3> attachments;
        if (samples == vk::SampleCountFlagBits::e1) {
            attachments = { swapchain_image_view, depth_image_view };
        }
        else {
            attachments = { color_image_view, depth_image_view, swapchain_image_view };
        }

        auto framebuffer_info
            = vk::FramebufferCreateInfo()
                  .setRenderPass(render_pass)
                  .setAttachmentCount(samples == vk::SampleCountFlagBits::e1 ? 2 : 3)
                  .setPAttachments(attachments.data())
                  .setWidth(swapchain_extent.width)
                  .setHeight(swapchain_extent.height)
                  .setLayers(1);

        vk::ResultValue<vk::Framebuffer> framebuffer_result
            = device.createFramebuffer(framebuffer_info, nullptr, loader);
        MVE_ASSERT(framebuffer_result.result == vk::Result::eSuccess, "[Renderer] Failed to create framebuffer")
        framebuffers.push_back(framebuffer_result.value);
    }
    return framebuffers;
}

inline std::vector<vk::CommandBuffer> create_vk_command_buffers(
    const vk::DispatchLoaderDynamic& loader,
    const vk::Device device,
    const vk::CommandPool command_pool,
    const int frames_in_flight)
{
    const auto buffer_alloc_info
        = vk::CommandBufferAllocateInfo()
              .setCommandPool(command_pool)
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandBufferCount(static_cast<uint32_t>(frames_in_flight));

    vk::ResultValue<std::vector<vk::CommandBuffer>> command_buffers_result
        = device.allocateCommandBuffers(buffer_alloc_info, loader);
    MVE_ASSERT(command_buffers_result.result == vk::Result::eSuccess, "[Renderer] Failed to allocate command buffers")
    return command_buffers_result.value;
}

inline Buffer create_buffer(
    const VmaAllocator allocator,
    const size_t size,
    const VkBufferUsageFlags usage,
    const VmaMemoryUsage memory_usage,
    const VmaAllocationCreateFlags flags = 0)
{
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;

    VmaAllocationCreateInfo vma_alloc_info = {};
    vma_alloc_info.usage = memory_usage;
    vma_alloc_info.flags = flags | VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;

    VkBuffer vk_buffer;
    VmaAllocation allocation;

    vmaCreateBuffer(allocator, &buffer_info, &vma_alloc_info, &vk_buffer, &allocation, nullptr);

    return { vk::Buffer(vk_buffer), allocation };
}

inline void cmd_copy_buffer(
    const vk::DispatchLoaderDynamic& loader,
    const vk::CommandBuffer command_buffer,
    const vk::Buffer src_buffer,
    const vk::Buffer dst_buffer,
    const vk::DeviceSize size)
{
    const auto copy_region = vk::BufferCopy().setSrcOffset(0).setDstOffset(0).setSize(size);
    command_buffer.copyBuffer(src_buffer, dst_buffer, 1, &copy_region, loader);
}

inline std::vector<FrameInFlight> create_frames_in_flight(
    const vk::DispatchLoaderDynamic& loader,
    const vk::Device device,
    const vk::CommandPool command_pool,
    const int frame_count)
{
    auto frames_in_flight = std::vector<FrameInFlight>();
    frames_in_flight.reserve(frame_count);

    const std::vector<vk::CommandBuffer> command_buffers
        = create_vk_command_buffers(loader, device, command_pool, frame_count);

    constexpr auto semaphore_info = vk::SemaphoreCreateInfo();
    constexpr auto fence_info = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (int i = 0; i < frame_count; i++) {
        FrameInFlight frame;
        const vk::ResultValue<vk::Semaphore> image_available_semaphore_result
            = device.createSemaphore(semaphore_info, nullptr, loader);
        MVE_ASSERT(
            image_available_semaphore_result.result == vk::Result::eSuccess,
            "[Renderer] Failed to create image available semaphore")
        frame.image_available_semaphore = image_available_semaphore_result.value;

        const vk::ResultValue<vk::Semaphore> render_finished_semaphore_result
            = device.createSemaphore(semaphore_info, nullptr, loader);
        MVE_ASSERT(
            render_finished_semaphore_result.result == vk::Result::eSuccess,
            "[Renderer] Failed to create render finished semaphore")
        frame.render_finished_semaphore = render_finished_semaphore_result.value;

        const vk::ResultValue<vk::Fence> in_flight_fence_result = device.createFence(fence_info, nullptr, loader);
        MVE_ASSERT(in_flight_fence_result.result == vk::Result::eSuccess, "[Renderer] Failed to create in flight fence")
        frame.in_flight_fence = in_flight_fence_result.value;
        frame.command_buffer = command_buffers.at(i);
        frame.uniform_buffers = {};
        frames_in_flight.push_back(frame);
    }

    return frames_in_flight;
}

inline vk::DescriptorSetLayout create_vk_descriptor_set_layout(
    const vk::DispatchLoaderDynamic& loader, const vk::Device device)
{
    constexpr auto ubo_layout_binding
        = vk::DescriptorSetLayoutBinding()
              .setBinding(0)
              .setDescriptorType(vk::DescriptorType::eUniformBuffer)
              .setDescriptorCount(1)
              .setStageFlags(vk::ShaderStageFlagBits::eAll)
              .setPImmutableSamplers(nullptr);

    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const auto layout_info = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&ubo_layout_binding);

    const vk::ResultValue<vk::DescriptorSetLayout> descriptor_set_layout_result
        = device.createDescriptorSetLayout(layout_info, nullptr, loader);
    MVE_ASSERT(
        descriptor_set_layout_result.result == vk::Result::eSuccess,
        "[Renderer] Failed to create descriptor set layout")
    return descriptor_set_layout_result.value;
}

inline vk::DescriptorPool create_vk_descriptor_pool(
    const vk::DispatchLoaderDynamic& loader, const vk::Device device, const int frames_in_flight)
{
    const auto pool_size = vk::DescriptorPoolSize()
                               .setType(vk::DescriptorType::eUniformBuffer)
                               .setDescriptorCount(static_cast<uint32_t>(frames_in_flight));

    const auto pool_info
        = vk::DescriptorPoolCreateInfo()
              .setPoolSizeCount(1)
              .setPPoolSizes(&pool_size)
              .setMaxSets(static_cast<uint32_t>(frames_in_flight));

    const vk::ResultValue<vk::DescriptorPool> descriptor_pool_result
        = device.createDescriptorPool(pool_info, nullptr, loader);
    MVE_ASSERT(descriptor_pool_result.result == vk::Result::eSuccess, "[Renderer] Failed to create descriptor pool")
    return descriptor_pool_result.value;
}

inline std::vector<vk::DescriptorSet> create_vk_descriptor_sets(
    const vk::DispatchLoaderDynamic& loader,
    const vk::Device device,
    const vk::DescriptorSetLayout layout,
    const vk::DescriptorPool pool,
    const int count)
{
    const auto layouts = std::vector(count, layout);

    const auto alloc_info
        = vk::DescriptorSetAllocateInfo()
              .setDescriptorPool(pool)
              .setDescriptorSetCount(static_cast<uint32_t>(count))
              .setPSetLayouts(layouts.data());

    vk::ResultValue<std::vector<vk::DescriptorSet>> descriptor_sets_result
        = device.allocateDescriptorSets(alloc_info, loader);
    MVE_ASSERT(descriptor_sets_result.result == vk::Result::eSuccess, "[Renderer] Failed to allocate descriptor sets")
    return descriptor_sets_result.value;
}

inline vk::CommandPool create_vk_command_pool(
    const vk::DispatchLoaderDynamic& loader, const vk::Device device, const QueueFamilyIndices queue_family_indices)
{
    assert(queue_family_indices.is_complete());

    const auto command_pool_info
        = vk::CommandPoolCreateInfo()
              .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
              .setQueueFamilyIndex(queue_family_indices.graphics_family.value());

    const vk::ResultValue<vk::CommandPool> command_pool_result
        = device.createCommandPool(command_pool_info, nullptr, loader);
    MVE_ASSERT(command_pool_result.result == vk::Result::eSuccess, "[Renderer] Failed to create command buffer")
    return command_pool_result.value;
}

inline Msaa vk_samples_to_msaa(const vk::SampleCountFlagBits vk_samples)
{
    switch (vk_samples) {
    case vk::SampleCountFlagBits::e1:
        return Msaa::samples_1;
    case vk::SampleCountFlagBits::e2:
        return Msaa::samples_2;
    case vk::SampleCountFlagBits::e4:
        return Msaa::samples_4;
    case vk::SampleCountFlagBits::e8:
        return Msaa::samples_8;
    case vk::SampleCountFlagBits::e16:
        return Msaa::samples_16;
    case vk::SampleCountFlagBits::e32:
        return Msaa::samples_32;
    case vk::SampleCountFlagBits::e64:
        return Msaa::samples_64;
    }
    MVE_ASSERT(false, "Unreachable");
}

}
