#include <mve/renderer.hpp>

#include <fstream>
#include <set>
#include <utility>
#include <vector>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "./logger.hpp"
#include <mve/common.hpp>
#include <mve/math/math.hpp>
#include <mve/vertex_data.hpp>
#include <mve/window.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace mve {
Renderer::Renderer(
    const Window& window,
    const std::string& app_name,
    int app_version_major,
    int app_version_minor,
    int app_version_patch)
    : c_frames_in_flight(2)
    , m_vk_instance(create_vk_instance(app_name, app_version_major, app_version_minor, app_version_patch))
    , m_resource_handle_count(0)
    , m_deferred_function_id_count(0)
{
    m_vk_loader = vk::DispatchLoaderDynamic(m_vk_instance, vkGetInstanceProcAddr);
#ifdef MVE_ENABLE_VALIDATION
    m_vk_debug_utils_messenger = create_vk_debug_messenger(m_vk_instance);
#endif
    m_vk_surface = create_vk_surface(m_vk_instance, window.glfw_handle());
    m_vk_physical_device = pick_vk_physical_device(m_vk_instance, m_vk_loader, m_vk_surface);
    m_msaa_samples = vk::SampleCountFlagBits::e1; // get_max_sample_count(m_vk_loader, m_vk_physical_device);
    m_vk_queue_family_indices = get_vk_queue_family_indices(m_vk_loader, m_vk_physical_device, m_vk_surface);
    m_vk_device = create_vk_logical_device(m_vk_loader, m_vk_physical_device, m_vk_queue_family_indices);
    m_vk_loader = vk::DispatchLoaderDynamic(m_vk_instance, vkGetInstanceProcAddr, m_vk_device, vkGetDeviceProcAddr);

    SwapchainSupportDetails swapchain_support_details
        = get_vk_swapchain_support_details(m_vk_loader, m_vk_physical_device, m_vk_surface);

    m_vk_swapchain_image_format = choose_vk_swapchain_surface_format(swapchain_support_details.formats);
    m_vk_swapchain_extent = get_vk_swapchain_extent(swapchain_support_details.capabilities, window.glfw_handle());
    m_vk_swapchain = create_vk_swapchain(
        m_vk_loader,
        m_vk_physical_device,
        m_vk_device,
        m_vk_surface,
        m_vk_swapchain_image_format,
        m_vk_swapchain_extent,
        m_vk_queue_family_indices);

    m_vk_swapchain_images = get_vk_swapchain_images(m_vk_loader, m_vk_device, m_vk_swapchain);

    m_vk_swapchain_image_views = create_vk_swapchain_image_views(
        m_vk_loader, m_vk_device, m_vk_swapchain_images, m_vk_swapchain_image_format.format);

    m_vk_graphics_queue = m_vk_device.getQueue(m_vk_queue_family_indices.graphics_family.value(), 0, m_vk_loader);
    m_vk_present_queue = m_vk_device.getQueue(m_vk_queue_family_indices.present_family.value(), 0, m_vk_loader);

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    VmaVulkanFunctions func = {};
    func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    allocatorCreateInfo.pVulkanFunctions = &func;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
    allocatorCreateInfo.physicalDevice = m_vk_physical_device;
    allocatorCreateInfo.device = m_vk_device;
    allocatorCreateInfo.instance = m_vk_instance;
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

    vmaCreateAllocator(&allocatorCreateInfo, &m_vma_allocator);

    m_vk_command_pool = create_vk_command_pool(m_vk_loader, m_vk_device, m_vk_queue_family_indices);

    m_color_image = create_color_image(
        m_vk_loader,
        m_vk_device,
        m_vma_allocator,
        m_vk_swapchain_extent,
        m_vk_swapchain_image_format.format,
        m_msaa_samples);
    m_depth_image = create_depth_image(
        m_vk_loader,
        m_vk_physical_device,
        m_vk_device,
        m_vk_command_pool,
        m_vk_graphics_queue,
        m_vma_allocator,
        m_vk_swapchain_extent,
        m_msaa_samples);

    m_vk_render_pass = create_vk_render_pass(
        m_vk_loader,
        m_vk_device,
        m_vk_swapchain_image_format.format,
        find_depth_format(m_vk_loader, m_vk_physical_device),
        m_msaa_samples);

    m_vk_render_pass_framebuffer = create_vk_render_pass_framebuffer(
        m_vk_loader,
        m_vk_device,
        m_vk_swapchain_image_format.format,
        find_depth_format(m_vk_loader, m_vk_physical_device),
        m_msaa_samples);

    m_vk_swapchain_framebuffers = create_vk_framebuffers(
        m_vk_loader,
        m_vk_device,
        m_vk_swapchain_image_views,
        m_vk_render_pass,
        m_vk_swapchain_extent,
        m_color_image.vk_image_view,
        m_depth_image.vk_image_view,
        m_msaa_samples);

    m_frames_in_flight = create_frames_in_flight(m_vk_loader, m_vk_device, m_vk_command_pool, c_frames_in_flight);

    m_current_draw_state.is_drawing = false;
    m_current_draw_state.frame_index = 0;
    m_current_draw_state.image_index = 0;
    m_current_draw_state.command_buffer = nullptr;
}

bool Renderer::has_validation_layer_support(const vk::DispatchLoaderDynamic& loader)
{
    vk::ResultValue<std::vector<vk::LayerProperties>> available_layers_result
        = vk::enumerateInstanceLayerProperties(loader);
    MVE_VAL_ASSERT(
        available_layers_result.result == vk::Result::eSuccess, "[Renderer] Failed to get validation layer properties")

    std::vector<vk::LayerProperties> available_layers = std::move(available_layers_result.value);

    const std::vector<const char*> validation_layers = get_vk_validation_layer_exts();

    for (const std::string& validation_layer : validation_layers) {
        bool layer_found = std::any_of(
            available_layers.cbegin(), available_layers.cend(), [&](const vk::LayerProperties& available_layer) {
                if (std::strlen(available_layer.layerName.data()) != validation_layer.size()) {
                    return false;
                }
                for (int i = 0; i < validation_layer.size(); i++) {
                    if (available_layer.layerName.at(i) != validation_layer.at(i)) {
                        return false;
                    }
                }
                return true;
            });
        if (!layer_found) {
            return false;
        }
    }
    return true;
}

vk::Instance Renderer::create_vk_instance(
    const std::string& app_name, int app_version_major, int app_version_minor, int app_version_patch)
{
    vk::DispatchLoaderDynamic temp_loader(vkGetInstanceProcAddr);
    MVE_VAL_ASSERT(
        has_validation_layer_support(temp_loader), "[Renderer] Validation layers requested but not available")

    auto application_info
        = vk::ApplicationInfo()
              .setPApplicationName(app_name.c_str())
              .setApplicationVersion(VK_MAKE_VERSION(app_version_major, app_version_minor, app_version_patch))
              .setPEngineName("Mini Vulkan Engine")
              .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
              .setApiVersion(VK_API_VERSION_1_1);

    std::vector<const char*> exts = get_vk_instance_required_exts();

#ifdef MVE_ENABLE_VALIDATION

    const std::vector<const char*> validation_layers = get_vk_validation_layer_exts();

    auto instance_create_info
        = vk::InstanceCreateInfo()
              .setPApplicationInfo(&application_info)
              .setEnabledExtensionCount(static_cast<uint32_t>(exts.size()))
              .setPpEnabledExtensionNames(exts.data())
              .setEnabledLayerCount(static_cast<uint32_t>(validation_layers.size()))
              .setPpEnabledLayerNames(validation_layers.data());
#else
    auto instance_create_info
        = vk::InstanceCreateInfo()
              .setPApplicationInfo(&application_info)
              .setEnabledExtensionCount(static_cast<uint32_t>(exts.size()))
              .setPpEnabledExtensionNames(exts.data())
              .setEnabledLayerCount(0);
#endif

    vk::ResultValue<vk::Instance> instance_result = vk::createInstance(instance_create_info, nullptr, temp_loader);
    MVE_ASSERT(instance_result.result == vk::Result::eSuccess, "[Renderer] Failed to create instance")
    return instance_result.value;
}

std::vector<const char*> Renderer::get_vk_validation_layer_exts()
{
    return std::vector<const char*> { "VK_LAYER_KHRONOS_validation" };
}

vk::PhysicalDevice Renderer::pick_vk_physical_device(
    vk::Instance instance, const vk::DispatchLoaderDynamic& loader, vk::SurfaceKHR surface)
{
    vk::ResultValue<std::vector<vk::PhysicalDevice>> physical_devices_result
        = instance.enumeratePhysicalDevices(loader);
    MVE_ASSERT(
        physical_devices_result.result == vk::Result::eSuccess, "[Renderer] Failed to get physical devices (GPUs)")

    std::vector<vk::PhysicalDevice> physical_devices = std::move(physical_devices_result.value);
    MVE_ASSERT(!physical_devices.empty(), "[Renderer] Failed to find Vulkan device")

    for (vk::PhysicalDevice physical_device : physical_devices) {
        log().debug("[Renderer] GPU Found: {0}", physical_device.getProperties(loader).deviceName);
    }

    for (vk::PhysicalDevice physical_device : physical_devices) {
        auto it
            = std::find_if(physical_devices.cbegin(), physical_devices.cend(), [&](vk::PhysicalDevice physical_device) {
                  return is_vk_physical_device_suitable(loader, physical_device, surface);
              });
        if (it != physical_devices.end()) {
            log().info("[Renderer] Using GPU: {0}", physical_device.getProperties(loader).deviceName);
            return physical_device;
        }
    }

    MVE_ASSERT(false, "[Renderer] Failed to find a suitable Vulkan device")
}

bool Renderer::is_vk_physical_device_suitable(
    const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
    QueueFamilyIndices indices = get_vk_queue_family_indices(loader, physical_device, surface);

    vk::ResultValue<std::vector<vk::ExtensionProperties>> available_exts_result
        = physical_device.enumerateDeviceExtensionProperties(nullptr, loader);
    MVE_ASSERT(
        available_exts_result.result == vk::Result::eSuccess, "[Renderer] Failed to get device extension properties")

    std::vector<vk::ExtensionProperties> available_exts = std::move(available_exts_result.value);

    std::vector<const char*> required_exts = get_vk_device_required_exts();

    for (const std::string& required_ext : required_exts) {
        bool is_available = std::any_of(
            available_exts.cbegin(), available_exts.cend(), [&](const vk::ExtensionProperties& ext_props) {
                return required_ext == ext_props.extensionName;
            });
        if (!is_available) {
            return false;
        }
    }

    SwapchainSupportDetails swapchain_support_details
        = get_vk_swapchain_support_details(loader, physical_device, surface);
    bool is_swapchain_adequate
        = !swapchain_support_details.formats.empty() && !swapchain_support_details.present_modes.empty();

    vk::PhysicalDeviceFeatures supported_features = physical_device.getFeatures(loader);

    return indices.is_complete() && is_swapchain_adequate && supported_features.samplerAnisotropy;
}

Renderer::QueueFamilyIndices Renderer::get_vk_queue_family_indices(
    const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queue_families = physical_device.getQueueFamilyProperties(loader);
    int i = 0;
    for (const vk::QueueFamilyProperties& queue_family : queue_families) {
        if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics_family = i;
        }

        vk::ResultValue<unsigned int> surface_support_result = physical_device.getSurfaceSupportKHR(i, surface, loader);
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
vk::Device Renderer::create_vk_logical_device(
    const vk::DispatchLoaderDynamic& loader,
    vk::PhysicalDevice physical_device,
    QueueFamilyIndices queue_family_indices)
{
    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

    assert(queue_family_indices.is_complete());

    std::set<uint32_t> _queue_families
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
              .setQueueCreateInfoCount(static_cast<uint32_t>(queue_create_infos.size()))
              .setPEnabledFeatures(&device_features)
              .setEnabledExtensionCount(static_cast<uint32_t>(required_exts.size()))
              .setPpEnabledExtensionNames(required_exts.data())
              .setEnabledLayerCount(validation_layers.size())
              .setPpEnabledLayerNames(validation_layers.data());
#else
    auto device_create_info
        = vk::DeviceCreateInfo()
              .setPQueueCreateInfos(queue_create_infos.data())
              .setQueueCreateInfoCount(static_cast<uint32_t>(queue_create_infos.size()))
              .setPEnabledFeatures(&device_features)
              .setEnabledExtensionCount(static_cast<uint32_t>(required_exts.size()))
              .setPpEnabledExtensionNames(required_exts.data())
              .setEnabledLayerCount(0);
#endif

    vk::ResultValue<vk::Device> device_result = physical_device.createDevice(device_create_info, nullptr, loader);
    MVE_ASSERT(device_result.result == vk::Result::eSuccess, "[Renderer] Failed to create device")
    return device_result.value;
}
vk::SurfaceKHR Renderer::create_vk_surface(vk::Instance instance, GLFWwindow* window)
{
    VkSurfaceKHR surface;
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    MVE_ASSERT(result == VK_SUCCESS, "[Renderer] Failed to create window surface")
    return vk::SurfaceKHR(surface);
}

std::vector<const char*> Renderer::get_vk_device_required_exts()
{
    return std::vector<const char*> { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}

Renderer::SwapchainSupportDetails Renderer::get_vk_swapchain_support_details(
    const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
    SwapchainSupportDetails details;

    vk::ResultValue<vk::SurfaceCapabilitiesKHR> capabilities_result
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
vk::SurfaceFormatKHR Renderer::choose_vk_swapchain_surface_format(
    const std::vector<vk::SurfaceFormatKHR>& available_formats)
{
    auto it = std::find_if(
        available_formats.cbegin(), available_formats.cend(), [](const vk::SurfaceFormatKHR& available_format) {
            return available_format.format == vk::Format::eB8G8R8A8Unorm
                && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });
    if (it != available_formats.end()) {
        return *it;
    }
    return available_formats[0];
}
vk::PresentModeKHR Renderer::choose_vk_swapchain_present_mode(
    const std::vector<vk::PresentModeKHR>& available_present_modes)
{
    auto it = std::find_if(
        available_present_modes.cbegin(),
        available_present_modes.cend(),
        [](const vk::PresentModeKHR& available_present_mode) {
            return available_present_mode == vk::PresentModeKHR::eMailbox;
        });
    if (it != available_present_modes.end()) {
        return *it;
    }
    return vk::PresentModeKHR::eFifo;
}
vk::Extent2D Renderer::get_vk_swapchain_extent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
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
}
vk::SwapchainKHR Renderer::create_vk_swapchain(
    const vk::DispatchLoaderDynamic& loader,
    vk::PhysicalDevice physical_device,
    vk::Device device,
    vk::SurfaceKHR surface,
    vk::SurfaceFormatKHR surface_format,
    vk::Extent2D surface_extent,
    QueueFamilyIndices queue_family_indices)
{
    SwapchainSupportDetails swapchain_support_details
        = get_vk_swapchain_support_details(loader, physical_device, surface);

    vk::PresentModeKHR present_mode = choose_vk_swapchain_present_mode(swapchain_support_details.present_modes);

    uint32_t image_count = swapchain_support_details.capabilities.minImageCount + 1;
    if (swapchain_support_details.capabilities.maxImageCount > 0
        && image_count > swapchain_support_details.capabilities.maxImageCount) {
        image_count = swapchain_support_details.capabilities.maxImageCount;
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
              .setPreTransform(swapchain_support_details.capabilities.currentTransform)
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setPresentMode(present_mode)
              .setClipped(true)
              .setOldSwapchain(nullptr);

    if (queue_family_indices.graphics_family != queue_family_indices.present_family) {
        uint32_t indices_arr[]
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

    vk::ResultValue<vk::SwapchainKHR> swapchain_result
        = device.createSwapchainKHR(swapchain_create_info, nullptr, loader);
    MVE_ASSERT(swapchain_result.result == vk::Result::eSuccess, "[Renderer] Failed to create swapchain")
    return swapchain_result.value;
}

std::vector<vk::Image> Renderer::get_vk_swapchain_images(
    const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::SwapchainKHR swapchain)
{
    vk::ResultValue<std::vector<vk::Image>> images_result = device.getSwapchainImagesKHR(swapchain, loader);
    MVE_ASSERT(images_result.result == vk::Result::eSuccess, "[Renderer] Failed to get swapchain images")
    return images_result.value;
}

std::vector<vk::ImageView> Renderer::create_vk_swapchain_image_views(
    const vk::DispatchLoaderDynamic& loader,
    vk::Device device,
    const std::vector<vk::Image>& swapchain_images,
    vk::Format image_format)
{
    std::vector<vk::ImageView> image_views;

    std::transform(
        swapchain_images.cbegin(),
        swapchain_images.cend(),
        std::back_inserter(image_views),
        [&](vk::Image swapchain_image) {
            return create_image_view(loader, device, swapchain_image, image_format, vk::ImageAspectFlagBits::eColor, 1);
        });

    return image_views;
}

vk::Pipeline Renderer::create_vk_graphics_pipeline(
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
              .setVertexAttributeDescriptionCount(static_cast<uint32_t>(attribute_descriptions.size()))
              .setPVertexAttributeDescriptions(attribute_descriptions.data());

    auto input_assembly_info
        = vk::PipelineInputAssemblyStateCreateInfo()
              .setTopology(vk::PrimitiveTopology::eTriangleList)
              .setPrimitiveRestartEnable(false);

    std::vector<vk::DynamicState> dynamic_states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

    auto dynamic_state_info = vk::PipelineDynamicStateCreateInfo()
                                  .setDynamicStateCount(static_cast<uint32_t>(dynamic_states.size()))
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

vk::PipelineLayout Renderer::create_vk_pipeline_layout(
    const vk::DispatchLoaderDynamic& loader, const std::vector<DescriptorSetLayoutHandleImpl>& layouts)
{
    std::vector<vk::DescriptorSetLayout> vk_layouts;
    std::transform(
        layouts.cbegin(), layouts.cend(), std::back_inserter(vk_layouts), [&](DescriptorSetLayoutHandleImpl handle) {
            return m_descriptor_set_layouts.at(handle);
        });

    auto pipeline_layout_info
        = vk::PipelineLayoutCreateInfo()
              .setSetLayouts(vk_layouts)
              .setPushConstantRangeCount(0)
              .setPPushConstantRanges(nullptr);

    vk::ResultValue<vk::PipelineLayout> pipeline_layout_result
        = m_vk_device.createPipelineLayout(pipeline_layout_info, nullptr, loader);
    MVE_ASSERT(pipeline_layout_result.result == vk::Result::eSuccess, "[Renderer] Failed to create pipline layout")
    return pipeline_layout_result.value;
}

vk::RenderPass Renderer::create_vk_render_pass_framebuffer(
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
        subpass.setPResolveAttachments(&(color_attachment_resolve_ref.value()));
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

vk::RenderPass Renderer::create_vk_render_pass(
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
        subpass.setPResolveAttachments(&(color_attachment_resolve_ref.value()));
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

std::vector<vk::Framebuffer> Renderer::create_vk_framebuffers(
    const vk::DispatchLoaderDynamic& loader,
    vk::Device device,
    const std::vector<vk::ImageView>& swapchain_image_views,
    vk::RenderPass render_pass,
    vk::Extent2D swapchain_extent,
    vk::ImageView color_image_view,
    vk::ImageView depth_image_view,
    vk::SampleCountFlagBits samples)
{
    std::vector<vk::Framebuffer> framebuffers;

    for (const vk::ImageView& swapchain_image_view : swapchain_image_views) {

        std::array<vk::ImageView, 3> attachments {};
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

vk::CommandPool Renderer::create_vk_command_pool(
    const vk::DispatchLoaderDynamic& loader, vk::Device device, QueueFamilyIndices queue_family_indices)
{
    assert(queue_family_indices.is_complete());

    auto command_pool_info = vk::CommandPoolCreateInfo()
                                 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                                 .setQueueFamilyIndex(queue_family_indices.graphics_family.value());

    vk::ResultValue<vk::CommandPool> command_pool_result = device.createCommandPool(command_pool_info, nullptr, loader);
    MVE_ASSERT(command_pool_result.result == vk::Result::eSuccess, "[Renderer] Failed to create command buffer")
    return command_pool_result.value;
}

std::vector<vk::CommandBuffer> Renderer::create_vk_command_buffers(
    const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::CommandPool command_pool, int frames_in_flight)
{
    auto buffer_alloc_info
        = vk::CommandBufferAllocateInfo()
              .setCommandPool(command_pool)
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandBufferCount(static_cast<uint32_t>(frames_in_flight));

    vk::ResultValue<std::vector<vk::CommandBuffer>> command_buffers_result
        = device.allocateCommandBuffers(buffer_alloc_info, loader);
    MVE_ASSERT(command_buffers_result.result == vk::Result::eSuccess, "[Renderer] Failed to allocate command buffers")
    return command_buffers_result.value;
}

Renderer::~Renderer()
{
#ifdef MVE_ENABLE_VALIDATION
    cleanup_vk_debug_messenger();
#endif
    vk::Result wait_result = m_vk_device.waitIdle(m_vk_loader);

    cleanup_vk_swapchain();

    for (std::optional<FramebufferImpl>& framebuffer : m_framebuffers) {
        if (framebuffer.has_value()) {
            for (vk::Framebuffer& buffer : framebuffer->vk_framebuffers) {
                m_vk_device.destroy(buffer, nullptr, m_vk_loader);
            }
            if (framebuffer->texture.is_valid()) {
                destroy(framebuffer->texture);
            }
        }
    }

    for (auto& [handle, texture] : m_textures) {
        m_vk_device.destroy(texture.vk_sampler, nullptr, m_vk_loader);
        m_vk_device.destroy(texture.vk_image_view, nullptr, m_vk_loader);
        vmaDestroyImage(m_vma_allocator, static_cast<VkImage>(texture.image.vk_handle), texture.image.vma_allocation);
    }

    m_descriptor_set_allocator.cleanup(m_vk_loader, m_vk_device);

    for (FrameInFlight& frame : m_frames_in_flight) {
        for (std::optional<UniformBufferImpl>& uniform_buffer : frame.uniform_buffers) {
            if (uniform_buffer.has_value()) {
                vmaUnmapMemory(m_vma_allocator, uniform_buffer->buffer.vma_allocation);
                vmaDestroyBuffer(
                    m_vma_allocator,
                    static_cast<VkBuffer>(uniform_buffer->buffer.vk_handle),
                    uniform_buffer->buffer.vma_allocation);
            }
        }
    }

    for (auto& [handle, layout] : m_descriptor_set_layouts) {
        m_vk_device.destroy(layout, nullptr, m_vk_loader);
    }

    for (std::optional<VertexBufferImpl>& vertex_buffer : m_vertex_buffers_new) {
        if (vertex_buffer.has_value()) {
            vmaDestroyBuffer(
                m_vma_allocator,
                static_cast<VkBuffer>(vertex_buffer->buffer.vk_handle),
                vertex_buffer->buffer.vma_allocation);
        }
    }

    for (std::optional<IndexBufferImpl>& index_buffer : m_index_buffers_new) {
        if (index_buffer.has_value()) {
            vmaDestroyBuffer(
                m_vma_allocator,
                static_cast<VkBuffer>(index_buffer->buffer.vk_handle),
                index_buffer->buffer.vma_allocation);
        }
    }

    vmaDestroyAllocator(m_vma_allocator);

    for (std::optional<GraphicsPipelineImpl>& pipeline : m_graphics_pipelines_new) {
        if (pipeline.has_value()) {
            m_vk_device.destroy(pipeline->pipeline, nullptr, m_vk_loader);
        }
    }

    for (std::optional<GraphicsPipelineLayoutImpl>& layout : m_graphics_pipeline_layouts_new) {
        if (layout.has_value()) {
            m_vk_device.destroy(layout->vk_handle, nullptr, m_vk_loader);
        }
    }

    m_vk_device.destroy(m_vk_render_pass, nullptr, m_vk_loader);
    m_vk_device.destroy(m_vk_render_pass_framebuffer, nullptr, m_vk_loader);

    for (FrameInFlight& frame : m_frames_in_flight) {
        m_vk_device.destroy(frame.render_finished_semaphore, nullptr, m_vk_loader);
        m_vk_device.destroy(frame.image_available_semaphore, nullptr, m_vk_loader);
        m_vk_device.destroy(frame.in_flight_fence, nullptr, m_vk_loader);
    }

    m_vk_device.destroy(m_vk_command_pool, nullptr, m_vk_loader);

    m_vk_device.destroy(nullptr, m_vk_loader);

    m_vk_instance.destroy(m_vk_surface, nullptr, m_vk_loader);
    m_vk_instance.destroy(nullptr, m_vk_loader);
}

void Renderer::recreate_swapchain(const Window& window)
{
    mve::Vector2i window_size;
    glfwGetFramebufferSize(window.glfw_handle(), &(window_size.x), &(window_size.y));

    while (window_size == mve::Vector2i(0, 0)) {
        glfwGetFramebufferSize(window.glfw_handle(), &(window_size.x), &(window_size.y));
        window.wait_for_events();
    }

    vk::Result wait_result = m_vk_device.waitIdle(m_vk_loader);
    MVE_ASSERT(wait_result == vk::Result::eSuccess, "[Renderer] Failed to wait idle for swapchain recreation")

    cleanup_vk_swapchain();

    SwapchainSupportDetails swapchain_support_details
        = get_vk_swapchain_support_details(m_vk_loader, m_vk_physical_device, m_vk_surface);

    m_vk_swapchain_extent = get_vk_swapchain_extent(swapchain_support_details.capabilities, window.glfw_handle());

    m_vk_swapchain = create_vk_swapchain(
        m_vk_loader,
        m_vk_physical_device,
        m_vk_device,
        m_vk_surface,
        m_vk_swapchain_image_format,
        m_vk_swapchain_extent,
        m_vk_queue_family_indices);

    m_vk_swapchain_images = get_vk_swapchain_images(m_vk_loader, m_vk_device, m_vk_swapchain);

    m_vk_swapchain_image_views = create_vk_swapchain_image_views(
        m_vk_loader, m_vk_device, m_vk_swapchain_images, m_vk_swapchain_image_format.format);

    m_color_image = create_color_image(
        m_vk_loader,
        m_vk_device,
        m_vma_allocator,
        m_vk_swapchain_extent,
        m_vk_swapchain_image_format.format,
        m_msaa_samples);

    m_depth_image = create_depth_image(
        m_vk_loader,
        m_vk_physical_device,
        m_vk_device,
        m_vk_command_pool,
        m_vk_graphics_queue,
        m_vma_allocator,
        m_vk_swapchain_extent,
        m_msaa_samples);

    m_vk_swapchain_framebuffers = create_vk_framebuffers(
        m_vk_loader,
        m_vk_device,
        m_vk_swapchain_image_views,
        m_vk_render_pass,
        m_vk_swapchain_extent,
        m_color_image.vk_image_view,
        m_depth_image.vk_image_view,
        m_msaa_samples);

    recreate_framebuffers();
}

void Renderer::cleanup_vk_swapchain()
{
    m_vk_device.destroy(m_color_image.vk_image_view, nullptr, m_vk_loader);
    vmaDestroyImage(
        m_vma_allocator, static_cast<VkImage>(m_color_image.image.vk_handle), m_color_image.image.vma_allocation);

    m_vk_device.destroy(m_depth_image.vk_image_view, nullptr, m_vk_loader);
    vmaDestroyImage(
        m_vma_allocator, static_cast<VkImage>(m_depth_image.image.vk_handle), m_depth_image.image.vma_allocation);

    for (vk::Framebuffer framebuffer : m_vk_swapchain_framebuffers) {
        m_vk_device.destroy(framebuffer, nullptr, m_vk_loader);
    }
    for (vk::ImageView image_view : m_vk_swapchain_image_views) {
        m_vk_device.destroy(image_view, nullptr, m_vk_loader);
    }
    m_vk_device.destroy(m_vk_swapchain, nullptr, m_vk_loader);
}

std::vector<const char*> Renderer::get_vk_instance_required_exts()
{
    uint32_t glfw_ext_count = 0;
    const char** glfw_exts;
    glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

    std::vector<const char*> exts(glfw_exts, glfw_exts + glfw_ext_count);

    exts.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifdef MVE_ENABLE_VALIDATION
    exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return exts;
}

vk::DebugUtilsMessengerEXT Renderer::create_vk_debug_messenger(vk::Instance instance)
{
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = vk_debug_callback;
    debug_create_info.pUserData = nullptr;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    MVE_VAL_ASSERT(func != nullptr, "[Renderer] Failed to create Vulkan debug messenger")

    VkDebugUtilsMessengerEXT debug_messenger;
    func(instance, &debug_create_info, nullptr, &debug_messenger);
    return vk::DebugUtilsMessengerEXT(debug_messenger);
}

VkBool32 Renderer::vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
    VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data_ptr)
{
    if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        log().warn("[Vulkan Debug] " + std::string(callback_data->pMessage));
    }

    return false;
}

void Renderer::cleanup_vk_debug_messenger()
{
    auto func
        = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vk_instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(m_vk_instance, static_cast<VkDebugUtilsMessengerEXT>(m_vk_debug_utils_messenger), nullptr);
    }
}

vk::VertexInputBindingDescription Renderer::create_vk_binding_description(const VertexLayout& layout)
{
    auto binding_description
        = vk::VertexInputBindingDescription()
              .setBinding(0)
              .setStride(get_vertex_layout_bytes(layout))
              .setInputRate(vk::VertexInputRate::eVertex);
    return binding_description;
}

std::vector<vk::VertexInputAttributeDescription> Renderer::create_vk_attribute_descriptions(const VertexLayout& layout)
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
            offset += sizeof(mve::Vector2);
            break;
        case VertexAttributeType::vec3:
            description.setFormat(vk::Format::eR32G32B32Sfloat);
            offset += sizeof(mve::Vector3);
            break;
        case VertexAttributeType::vec4:
            description.setFormat(vk::Format::eR32G32B32A32Sfloat);
            offset += sizeof(mve::Vector4);
            break;
        }

        attribute_descriptions.push_back(description);
    }

    return attribute_descriptions;
}

std::vector<Renderer::FrameInFlight> Renderer::create_frames_in_flight(
    const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::CommandPool command_pool, int frame_count)
{
    auto frames_in_flight = std::vector<FrameInFlight>();
    frames_in_flight.reserve(frame_count);

    std::vector<vk::CommandBuffer> command_buffers
        = create_vk_command_buffers(loader, device, command_pool, frame_count);

    auto semaphore_info = vk::SemaphoreCreateInfo();
    auto fence_info = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (int i = 0; i < frame_count; i++) {
        FrameInFlight frame;
        vk::ResultValue<vk::Semaphore> image_available_semaphore_result
            = device.createSemaphore(semaphore_info, nullptr, loader);
        MVE_ASSERT(
            image_available_semaphore_result.result == vk::Result::eSuccess,
            "[Renderer] Failed to create image available semaphore")
        frame.image_available_semaphore = image_available_semaphore_result.value;

        vk::ResultValue<vk::Semaphore> render_finished_semaphore_result
            = device.createSemaphore(semaphore_info, nullptr, loader);
        MVE_ASSERT(
            render_finished_semaphore_result.result == vk::Result::eSuccess,
            "[Renderer] Failed to create render finished semaphore")
        frame.render_finished_semaphore = render_finished_semaphore_result.value;

        vk::ResultValue<vk::Fence> in_flight_fence_result = device.createFence(fence_info, nullptr, loader);
        MVE_ASSERT(in_flight_fence_result.result == vk::Result::eSuccess, "[Renderer] Failed to create in flight fence")
        frame.in_flight_fence = in_flight_fence_result.value;
        frame.command_buffer = command_buffers.at(i);
        frame.uniform_buffers = {};
        frames_in_flight.push_back(frame);
    }

    return frames_in_flight;
}

void Renderer::destroy(VertexBuffer& vertex_buffer)
{
    MVE_VAL_ASSERT(vertex_buffer.is_valid(), "[Renderer] Attempted to destroy invalid vertex buffer")
    log().debug("[Renderer] Destroyed vertex buffer with ID: {}", vertex_buffer.handle());
    size_t handle = vertex_buffer.handle();
    vertex_buffer.invalidate();
    defer_after_all_frames([this, handle](uint32_t) {
        vmaDestroyBuffer(
            m_vma_allocator,
            static_cast<VkBuffer>(m_vertex_buffers_new.at(handle)->buffer.vk_handle),
            m_vertex_buffers_new.at(handle)->buffer.vma_allocation);
        m_vertex_buffers_new[handle].reset();
    });
}

Renderer::Buffer Renderer::create_buffer(
    VmaAllocator allocator,
    size_t size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memory_usage,
    VmaAllocationCreateFlags flags)
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

void Renderer::cmd_copy_buffer(
    vk::DispatchLoaderDynamic& loader,
    vk::CommandBuffer command_buffer,
    vk::Buffer src_buffer,
    vk::Buffer dst_buffer,
    vk::DeviceSize size)
{
    auto copy_region = vk::BufferCopy().setSrcOffset(0).setDstOffset(0).setSize(size);
    command_buffer.copyBuffer(src_buffer, dst_buffer, 1, &copy_region, loader);
}

void Renderer::begin_render_pass_present()
{
    auto clear_color = vk::ClearValue(vk::ClearColorValue(std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f }));

    std::array<vk::ClearValue, 2> clear_values {};
    clear_values[0].setColor(vk::ClearColorValue(std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f }));
    clear_values[1].setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));

    auto render_pass_begin_info
        = vk::RenderPassBeginInfo()
              .setRenderPass(m_vk_render_pass)
              .setFramebuffer(m_vk_swapchain_framebuffers[m_current_draw_state.image_index])
              .setRenderArea(vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_vk_swapchain_extent))
              .setClearValueCount(static_cast<uint32_t>(clear_values.size()))
              .setPClearValues(clear_values.data());

    m_current_draw_state.command_buffer.beginRenderPass(
        render_pass_begin_info, vk::SubpassContents::eInline, m_vk_loader);

    auto viewport
        = vk::Viewport()
              .setX(0.0f)
              .setY(0.0f)
              .setWidth(static_cast<float>(m_vk_swapchain_extent.width))
              .setHeight(static_cast<float>(m_vk_swapchain_extent.height))
              .setMinDepth(0.0f)
              .setMaxDepth(1.0f);

    m_current_draw_state.command_buffer.setViewport(0, { viewport }, m_vk_loader);

    auto scissor = vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_vk_swapchain_extent);

    m_current_draw_state.command_buffer.setScissor(0, { scissor }, m_vk_loader);
}

void Renderer::begin_render_pass_framebuffer(Framebuffer& framebuffer)
{
    auto clear_color
        = vk::ClearColorValue(std::array<float, 4> { 142.0f / 255.0f, 186.0f / 255.0f, 255.0f / 255.0f, 1.0f });

    std::array<vk::ClearValue, 2> clear_values {};
    clear_values[0].setColor(clear_color);
    clear_values[1].setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));

    auto render_pass_begin_info
        = vk::RenderPassBeginInfo()
              .setRenderPass(m_vk_render_pass_framebuffer)
              .setFramebuffer(m_framebuffers[framebuffer.m_handle]->vk_framebuffers[m_current_draw_state.image_index])
              .setRenderArea(vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_vk_swapchain_extent))
              .setClearValueCount(static_cast<uint32_t>(clear_values.size()))
              .setPClearValues(clear_values.data());

    m_current_draw_state.command_buffer.beginRenderPass(
        render_pass_begin_info, vk::SubpassContents::eInline, m_vk_loader);

    auto viewport
        = vk::Viewport()
              .setX(0.0f)
              .setY(0.0f)
              .setWidth(static_cast<float>(m_vk_swapchain_extent.width))
              .setHeight(static_cast<float>(m_vk_swapchain_extent.height))
              .setMinDepth(0.0f)
              .setMaxDepth(1.0f);

    m_current_draw_state.command_buffer.setViewport(0, { viewport }, m_vk_loader);

    auto scissor = vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_vk_swapchain_extent);

    m_current_draw_state.command_buffer.setScissor(0, { scissor }, m_vk_loader);
}

void Renderer::begin_frame(const Window& window)
{
    MVE_VAL_ASSERT(!m_current_draw_state.is_drawing, "[Renderer] Already drawing")

    m_current_draw_state.is_drawing = true;

    FrameInFlight& frame = m_frames_in_flight[m_current_draw_state.frame_index];

    wait_ready();

    vk::ResultValue<uint32_t> acquire_result = m_vk_device.acquireNextImageKHR(
        m_vk_swapchain, UINT64_MAX, frame.image_available_semaphore, nullptr, m_vk_loader);
    if (acquire_result.result == vk::Result::eSuboptimalKHR) {
        recreate_swapchain(window);
        m_current_draw_state.is_drawing = false;
        return;
    }
    else {
        MVE_ASSERT(acquire_result.result == vk::Result::eSuccess, "[Renderer] Failed to acquire swapchain image")
    }
    m_current_draw_state.image_index = acquire_result.value;

    vmaSetCurrentFrameIndex(m_vma_allocator, acquire_result.value);

    m_vk_device.resetFences({ frame.in_flight_fence }, m_vk_loader);

    frame.command_buffer.reset(vk::CommandBufferResetFlags(), m_vk_loader);

    while (!frame.funcs.empty()) {
        auto& def = m_deferred_functions.at(frame.funcs.front());
        std::invoke(def.function, m_current_draw_state.frame_index);
        def.counter--;
        if (def.counter <= 0) {
            m_deferred_functions.erase(frame.funcs.front());
        }
        frame.funcs.pop();
    }

    std::queue<uint32_t> continue_defer;
    while (!m_wait_frames_deferred_functions.empty()) {
        uint32_t id = m_wait_frames_deferred_functions.front();
        m_wait_frames_deferred_functions.pop();
        auto& def = m_deferred_functions.at(id);
        def.counter--;
        if (def.counter <= 0) {
            std::invoke(def.function, m_current_draw_state.frame_index);
            m_deferred_functions.erase(id);
        }
        else {
            continue_defer.push(id);
        }
    }
    m_wait_frames_deferred_functions = std::move(continue_defer);

    for (size_t i = 0; i < m_deferred_descriptor_writes.size(); i++) {
        DeferredDescriptorWriteData& write_data = m_deferred_descriptor_writes[i];
        vk::DescriptorBufferInfo buffer_info;
        vk::DescriptorImageInfo image_info;
        vk::WriteDescriptorSet descriptor_write;
        switch (write_data.data_type) {
        case DescriptorBindingType::uniform_buffer:
            buffer_info
                = vk::DescriptorBufferInfo()
                      .setBuffer(m_frames_in_flight[m_current_draw_state.frame_index]
                                     .uniform_buffers[write_data.data_handle]
                                     ->buffer.vk_handle)
                      .setOffset(0)
                      .setRange(m_frames_in_flight[m_current_draw_state.frame_index]
                                    .uniform_buffers[write_data.data_handle]
                                    ->size);

            descriptor_write
                = vk::WriteDescriptorSet()
                      .setDstSet(m_frames_in_flight[m_current_draw_state.frame_index]
                                     .descriptor_sets[write_data.descriptor_handle]
                                     ->vk_handle)
                      .setDstBinding(write_data.binding)
                      .setDstArrayElement(0)
                      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                      .setDescriptorCount(1)
                      .setPBufferInfo(&buffer_info);

            m_vk_device.updateDescriptorSets(1, &descriptor_write, 0, nullptr, m_vk_loader);
            break;
        case DescriptorBindingType::texture:
            image_info = vk::DescriptorImageInfo()
                             .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                             .setImageView(m_textures[write_data.data_handle].vk_image_view)
                             .setSampler(m_textures[write_data.data_handle].vk_sampler);

            descriptor_write
                = vk::WriteDescriptorSet()
                      .setDstSet(m_frames_in_flight[m_current_draw_state.frame_index]
                                     .descriptor_sets[write_data.descriptor_handle]
                                     ->vk_handle)
                      .setDstBinding(write_data.binding)
                      .setDstArrayElement(0)
                      .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                      .setDescriptorCount(1)
                      .setPImageInfo(&image_info);

            m_vk_device.updateDescriptorSets(1, &descriptor_write, 0, nullptr, m_vk_loader);
            break;
        }
        write_data.counter--;
        if (write_data.counter <= 0) {
            m_deferred_descriptor_writes.erase(m_deferred_descriptor_writes.begin() + static_cast<long long>(i));
            i--;
        }
    }
    for (size_t i = 0; i < m_deferred_uniform_updates.size(); i++) {
        DeferredUniformUpdateData& update_data = m_deferred_uniform_updates[i];
        update_uniform(
            update_data.handle,
            update_data.location,
            update_data.data.data(),
            update_data.data_size,
            m_current_draw_state.frame_index);
        update_data.counter--;
        if (update_data.counter <= 0) {
            m_deferred_uniform_updates.erase(m_deferred_uniform_updates.begin() + static_cast<long long>(i));
            i--;
        }
    }

    m_current_draw_state.command_buffer = m_frames_in_flight[m_current_draw_state.frame_index].command_buffer;

    auto buffer_begin_info = vk::CommandBufferBeginInfo();
    vk::Result begin_result = m_current_draw_state.command_buffer.begin(buffer_begin_info, m_vk_loader);
    MVE_ASSERT(begin_result == vk::Result::eSuccess, "[Renderer] Failed to begin command buffer recording")

    while (!m_command_buffer_deferred_functions.empty()) {
        std::invoke(m_command_buffer_deferred_functions.front(), m_current_draw_state.command_buffer);
        m_command_buffer_deferred_functions.pop();
    }
}

void Renderer::end_frame(const Window& window)
{
    vk::Result end_result = m_current_draw_state.command_buffer.end(m_vk_loader);
    MVE_ASSERT(end_result == vk::Result::eSuccess, "[Renderer] Failed to end command buffer recording")

    FrameInFlight& frame = m_frames_in_flight[m_current_draw_state.frame_index];

    vk::Semaphore wait_semaphores[] = { frame.image_available_semaphore };
    vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::Semaphore signal_semaphores[] = { frame.render_finished_semaphore };

    auto submit_info
        = vk::SubmitInfo()
              .setWaitSemaphores(wait_semaphores)
              .setWaitDstStageMask(wait_stages)
              .setCommandBufferCount(1)
              .setPCommandBuffers(&(frame.command_buffer))
              .setSignalSemaphores(signal_semaphores);

    vk::Result graphics_submit_result = m_vk_graphics_queue.submit({ submit_info }, frame.in_flight_fence, m_vk_loader);
    MVE_ASSERT(graphics_submit_result == vk::Result::eSuccess, "[Renderer] Failed to submit to graphics queue")

    vk::SwapchainKHR swapchains[] = { m_vk_swapchain };

    auto present_info
        = vk::PresentInfoKHR()
              .setWaitSemaphores(signal_semaphores)
              .setSwapchains(swapchains)
              .setPImageIndices(&(m_current_draw_state.image_index));

    vk::Result present_result = m_vk_present_queue.presentKHR(present_info, m_vk_loader);
    if (present_result == vk::Result::eSuboptimalKHR || present_result == vk::Result::eErrorOutOfDateKHR) {
        recreate_swapchain(window);
    }
    else {
        MVE_ASSERT(present_result == vk::Result::eSuccess, "[Renderer] Failed to present frame")
    }

    m_current_draw_state.frame_index = (m_current_draw_state.frame_index + 1) % c_frames_in_flight;

    m_current_draw_state.is_drawing = false;
}

vk::DescriptorSetLayout Renderer::create_vk_descriptor_set_layout(
    const vk::DispatchLoaderDynamic& loader, vk::Device device)
{
    auto ubo_layout_binding
        = vk::DescriptorSetLayoutBinding()
              .setBinding(0)
              .setDescriptorType(vk::DescriptorType::eUniformBuffer)
              .setDescriptorCount(1)
              .setStageFlags(vk::ShaderStageFlagBits::eAll)
              .setPImmutableSamplers(nullptr);

    auto layout_info = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&ubo_layout_binding);

    vk::ResultValue<vk::DescriptorSetLayout> descriptor_set_layout_result
        = device.createDescriptorSetLayout(layout_info, nullptr, loader);
    MVE_ASSERT(
        descriptor_set_layout_result.result == vk::Result::eSuccess,
        "[Renderer] Failed to create descriptor set layout")
    return descriptor_set_layout_result.value;
}

vk::DescriptorPool Renderer::create_vk_descriptor_pool(
    const vk::DispatchLoaderDynamic& loader, vk::Device device, int frames_in_flight)
{
    auto pool_size = vk::DescriptorPoolSize()
                         .setType(vk::DescriptorType::eUniformBuffer)
                         .setDescriptorCount(static_cast<uint32_t>(frames_in_flight));

    auto pool_info = vk::DescriptorPoolCreateInfo()
                         .setPoolSizeCount(1)
                         .setPPoolSizes(&pool_size)
                         .setMaxSets(static_cast<uint32_t>(frames_in_flight));

    vk::ResultValue<vk::DescriptorPool> descriptor_pool_result
        = device.createDescriptorPool(pool_info, nullptr, loader);
    MVE_ASSERT(descriptor_pool_result.result == vk::Result::eSuccess, "[Renderer] Failed to create descriptor pool")
    return descriptor_pool_result.value;
}

std::vector<vk::DescriptorSet> Renderer::create_vk_descriptor_sets(
    const vk::DispatchLoaderDynamic& loader,
    vk::Device device,
    vk::DescriptorSetLayout layout,
    vk::DescriptorPool pool,
    int count)
{
    auto layouts = std::vector<vk::DescriptorSetLayout>(count, layout);

    auto alloc_info = vk::DescriptorSetAllocateInfo()
                          .setDescriptorPool(pool)
                          .setDescriptorSetCount(static_cast<uint32_t>(count))
                          .setPSetLayouts(layouts.data());

    vk::ResultValue<std::vector<vk::DescriptorSet>> descriptor_sets_result
        = device.allocateDescriptorSets(alloc_info, loader);
    MVE_ASSERT(descriptor_sets_result.result == vk::Result::eSuccess, "[Renderer] Failed to allocate descriptor sets")
    return descriptor_sets_result.value;
}

mve::Vector2i Renderer::extent() const
{
    return { static_cast<int>(m_vk_swapchain_extent.width), static_cast<int>(m_vk_swapchain_extent.height) };
}

void Renderer::wait_ready()
{
    FrameInFlight& frame = m_frames_in_flight[m_current_draw_state.frame_index];

    vk::Result fence_wait_result = m_vk_device.waitForFences(frame.in_flight_fence, true, UINT64_MAX, m_vk_loader);
    MVE_ASSERT(fence_wait_result == vk::Result::eSuccess, "[Renderer] Failed waiting for frame (fences)")
}

void Renderer::update_uniform(
    size_t handle, UniformLocation location, void* data_ptr, size_t size, uint32_t frame_index)
{
    UniformBufferImpl& buffer = *(m_frames_in_flight.at(frame_index).uniform_buffers.at(handle));
    memcpy(&(buffer.mapped_ptr[location.value()]), data_ptr, size);
}

Renderer::DescriptorSetLayoutHandleImpl Renderer::create_descriptor_set_layout(
    const vk::DispatchLoaderDynamic& loader, uint32_t set, const Shader& vertex_shader, const Shader& fragment_shader)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    if (vertex_shader.has_descriptor_set(set)) {
        const ShaderDescriptorSet& vertex_set = vertex_shader.descriptor_set(set);
        for (const auto& [binding_num, shader_binding] : vertex_set.bindings()) {
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
            bindings.push_back(binding);
        }
    }
    if (fragment_shader.has_descriptor_set(set)) {
        const ShaderDescriptorSet& fragment_set = fragment_shader.descriptor_set(set);
        for (const auto& [binding_num, fragment_binding] : fragment_set.bindings()) {
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
            bindings.push_back(binding);
        }
    }

    auto layout_info = vk::DescriptorSetLayoutCreateInfo().setBindings(bindings);

    vk::ResultValue<vk::DescriptorSetLayout> descriptor_set_layout_result
        = m_vk_device.createDescriptorSetLayout(layout_info, nullptr, loader);
    MVE_ASSERT(
        descriptor_set_layout_result.result == vk::Result::eSuccess,
        "[Renderer] Failed to create descriptor set layout")
    vk::DescriptorSetLayout vk_layout = descriptor_set_layout_result.value;

    auto handle = DescriptorSetLayoutHandleImpl(m_resource_handle_count);
    m_descriptor_set_layouts.insert({ handle, vk_layout });
    m_resource_handle_count++;

    log().debug("[Renderer] Descriptor set layout created with ID: {}", handle);

    return handle;
}

size_t Renderer::create_graphics_pipeline_layout(
    const vk::DispatchLoaderDynamic& loader, const Shader& vertex_shader, const Shader& fragment_shader)
{
    std::vector<DescriptorSetLayoutHandleImpl> layouts;
    std::unordered_map<uint64_t, DescriptorSetLayoutHandleImpl> descriptor_set_layouts;

    for (uint32_t i = 0; i <= 3; i++) {
        if (vertex_shader.has_descriptor_set(i) || fragment_shader.has_descriptor_set(i)) {
            DescriptorSetLayoutHandleImpl descriptor_set_layout
                = create_descriptor_set_layout(loader, i, vertex_shader, fragment_shader);
            layouts.push_back(descriptor_set_layout);
            descriptor_set_layouts.insert({ i, descriptor_set_layout });
        }
    }

    vk::PipelineLayout vk_layout = create_vk_pipeline_layout(m_vk_loader, layouts);

    std::optional<size_t> id;
    for (size_t i = 0; i < m_graphics_pipeline_layouts_new.size(); i++) {
        if (!m_graphics_pipeline_layouts_new[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = m_graphics_pipeline_layouts_new.size();
        m_graphics_pipeline_layouts_new.emplace_back();
    }
    m_graphics_pipeline_layouts_new[*id]
        = { .vk_handle = vk_layout, .descriptor_set_layouts = std::move(descriptor_set_layouts) };

    log().debug("[Renderer] Graphics pipeline layout created with ID: {}", *id);

    return *id;
}

void Renderer::defer_to_all_frames(std::function<void(uint32_t)> func)
{
    uint32_t id = m_deferred_function_id_count;
    m_deferred_function_id_count++;
    m_deferred_functions.insert({ id, { std::move(func), c_frames_in_flight } });
    for (FrameInFlight& frame : m_frames_in_flight) {
        frame.funcs.push(id);
    }
}

void Renderer::defer_to_next_frame(std::function<void(uint32_t)> func)
{
    uint32_t id = m_deferred_function_id_count;
    m_deferred_function_id_count++;
    m_deferred_functions.insert({ id, { std::move(func), 1 } });
    m_frames_in_flight.at(m_current_draw_state.frame_index).funcs.push(id);
}

void Renderer::resize(const Window& window)
{
    recreate_swapchain(window);
}

void Renderer::defer_after_all_frames(std::function<void(uint32_t)> func)
{
    uint32_t id = m_deferred_function_id_count;
    m_deferred_function_id_count++;
    m_deferred_functions.insert({ id, { std::move(func), c_frames_in_flight } });
    m_wait_frames_deferred_functions.push(id);
}

vk::CommandBuffer Renderer::begin_single_submit(
    const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::CommandPool pool)
{
    auto command_buffer_alloc_info
        = vk::CommandBufferAllocateInfo()
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandPool(pool)
              .setCommandBufferCount(1);

    vk::ResultValue<std::vector<vk::CommandBuffer>> command_buffer_result
        = device.allocateCommandBuffers(command_buffer_alloc_info, loader);
    MVE_ASSERT(
        command_buffer_result.result == vk::Result::eSuccess, "[Renderer] Failed to allocate texture command buffer")
    vk::CommandBuffer command_buffer = command_buffer_result.value.at(0);

    auto begin_info = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    vk::Result begin_result = command_buffer.begin(begin_info, loader);
    MVE_ASSERT(begin_result == vk::Result::eSuccess, "[Renderer] Failed to begin single submit buffer")

    return command_buffer;
}

void Renderer::end_single_submit(
    const vk::DispatchLoaderDynamic& loader,
    vk::Device device,
    vk::CommandPool pool,
    vk::CommandBuffer command_buffer,
    vk::Queue queue)
{
    vk::Result end_result = command_buffer.end(loader);
    MVE_ASSERT(end_result == vk::Result::eSuccess, "[Renderer] Failed to end single submit buffer")

    auto submit_info = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&command_buffer);

    vk::Result submit_result = queue.submit(1, &submit_info, VK_NULL_HANDLE, loader);
    MVE_ASSERT(submit_result == vk::Result::eSuccess, "[Renderer] Failed to submit single submit buffer")
    vk::Result wait_result = queue.waitIdle(loader);
    MVE_ASSERT(wait_result == vk::Result::eSuccess, "[Renderer] Failed to wait for queue for single submit")

    device.freeCommandBuffers(pool, 1, &command_buffer, loader);
}

void Renderer::cmd_transition_image_layout(
    const vk::DispatchLoaderDynamic& loader,
    vk::CommandBuffer command_buffer,
    vk::Image image,
    vk::Format format,
    vk::ImageLayout old_layout,
    vk::ImageLayout new_layout,
    uint32_t mip_levels)
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

void Renderer::cmd_copy_buffer_to_image(
    const vk::DispatchLoaderDynamic& loader,
    vk::CommandBuffer command_buffer,
    vk::Buffer buffer,
    vk::Image image,
    uint32_t width,
    uint32_t height)
{
    auto region
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

vk::ImageView Renderer::create_image_view(
    const vk::DispatchLoaderDynamic& loader,
    vk::Device device,
    vk::Image image,
    vk::Format format,
    vk::ImageAspectFlags aspect_flags,
    uint32_t mip_levels)
{
    auto components
        = vk::ComponentMapping()
              .setR(vk::ComponentSwizzle::eIdentity)
              .setG(vk::ComponentSwizzle::eIdentity)
              .setB(vk::ComponentSwizzle::eIdentity)
              .setA(vk::ComponentSwizzle::eIdentity);

    auto image_subresource_range
        = vk::ImageSubresourceRange()
              .setAspectMask(aspect_flags)
              .setBaseMipLevel(0)
              .setLevelCount(mip_levels)
              .setBaseArrayLayer(0)
              .setLayerCount(1);

    auto view_info
        = vk::ImageViewCreateInfo()
              .setImage(image)
              .setViewType(vk::ImageViewType::e2D)
              .setFormat(format)
              .setComponents(components)
              .setSubresourceRange(image_subresource_range);

    vk::ResultValue<vk::ImageView> image_view_result = device.createImageView(view_info, nullptr, loader);
    MVE_ASSERT(image_view_result.result == vk::Result::eSuccess, "[Renderer] Failed to create image view")
    return image_view_result.value;
}

vk::Sampler Renderer::create_texture_sampler(
    const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device, vk::Device device, uint32_t mip_levels)
{
    auto sampler_info
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

    vk::ResultValue<vk::Sampler> sampler_result = device.createSampler(sampler_info, nullptr, loader);
    MVE_ASSERT(sampler_result.result == vk::Result::eSuccess, "[Renderer] Failed to create image sampler")
    return sampler_result.value;
}

Renderer::DepthImage Renderer::create_depth_image(
    const vk::DispatchLoaderDynamic& loader,
    vk::PhysicalDevice physical_device,
    vk::Device device,
    vk::CommandPool pool,
    vk::Queue queue,
    VmaAllocator allocator,
    vk::Extent2D extent,
    vk::SampleCountFlagBits samples)
{
    vk::Format depth_format = find_depth_format(loader, physical_device);

    Image depth_image = create_image(
        allocator,
        extent.width,
        extent.height,
        1,
        samples,
        depth_format,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        true);

    vk::ImageView depth_image_view
        = create_image_view(loader, device, depth_image.vk_handle, depth_format, vk::ImageAspectFlagBits::eDepth, 1);

    vk::CommandBuffer command_buffer = begin_single_submit(loader, device, pool);

    cmd_transition_image_layout(
        loader,
        command_buffer,
        depth_image.vk_handle,
        depth_format,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal,
        1);

    end_single_submit(loader, device, pool, command_buffer, queue);

    return { depth_image, depth_image_view };
}

vk::Format Renderer::find_supported_format(
    const vk::DispatchLoaderDynamic& loader,
    vk::PhysicalDevice physical_device,
    const std::vector<vk::Format>& formats,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlags features)
{
    for (vk::Format format : formats) {
        vk::FormatProperties properties = physical_device.getFormatProperties(format, loader);
        if ((tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features)
            || (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)) {
            return format;
        }
    }
    MVE_ASSERT(false, "[Renderer] Failed to find supported format")
}

vk::Format Renderer::find_depth_format(const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device)
{
    const std::vector<vk::Format> formats
        = { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };
    return find_supported_format(
        loader,
        physical_device,
        formats,
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool Renderer::has_stencil_component(vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

Renderer::Image Renderer::create_image(
    VmaAllocator allocator,
    uint32_t width,
    uint32_t height,
    uint32_t mip_levels,
    vk::SampleCountFlagBits samples,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    bool dedicated)
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

void Renderer::cmd_generate_mipmaps(
    const vk::DispatchLoaderDynamic& loader,
    vk::PhysicalDevice physical_device,
    vk::CommandBuffer command_buffer,
    vk::Image image,
    vk::Format format,
    uint32_t width,
    uint32_t height,
    uint32_t mip_levels)
{
    // Check image format supports linear blitting
    vk::FormatProperties properties = physical_device.getFormatProperties(format, loader);
    MVE_VAL_ASSERT(
        properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear,
        "[Renderer] Image format does not support linear blitting for mip-mapping")

    auto subresource_range
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

        std::array<vk::Offset3D, 2> src_offsets
            = { vk::Offset3D(0, 0, 0),
                vk::Offset3D(static_cast<int32_t>(mip_width), static_cast<int32_t>(mip_height), 1) };

        auto src_subresource
            = vk::ImageSubresourceLayers()
                  .setAspectMask(vk::ImageAspectFlagBits::eColor)
                  .setMipLevel(i - 1)
                  .setBaseArrayLayer(0)
                  .setLayerCount(1);

        std::array<vk::Offset3D, 2> dst_offsets
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

vk::SampleCountFlagBits Renderer::get_max_sample_count(
    const vk::DispatchLoaderDynamic& loader, vk::PhysicalDevice physical_device)
{
    vk::PhysicalDeviceProperties properties = physical_device.getProperties(loader);

    vk::SampleCountFlags counts
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

Renderer::RenderImage Renderer::create_color_image(
    const vk::DispatchLoaderDynamic& loader,
    vk::Device device,
    VmaAllocator allocator,
    vk::Extent2D swapchain_extent,
    vk::Format swapchain_format,
    vk::SampleCountFlagBits samples)
{
    Image color_image = create_image(
        allocator,
        swapchain_extent.width,
        swapchain_extent.height,
        1,
        samples,
        swapchain_format,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        true); // TODO: make the sampled optional

    vk::ImageView image_view = create_image_view(
        loader, device, color_image.vk_handle, swapchain_format, vk::ImageAspectFlagBits::eColor, 1);

    return { color_image, image_view };
}

void Renderer::defer_to_command_buffer_front(const std::function<void(vk::CommandBuffer)>& func)
{
    m_command_buffer_deferred_functions.push(func);
}

void Renderer::write_descriptor_binding(
    const DescriptorSet& descriptor_set, const ShaderDescriptorBinding& descriptor_binding, const Texture& texture)
{
    DeferredDescriptorWriteData write_data {
        .counter = c_frames_in_flight,
        .data_type = DescriptorBindingType::texture,
        .data_handle = texture.m_handle,
        .descriptor_handle = descriptor_set.m_handle,
        .binding = descriptor_binding.binding()
    };
    m_deferred_descriptor_writes.push_back(write_data);
}

VertexBuffer Renderer::create_vertex_buffer(const VertexData& vertex_data)
{
    size_t buffer_size = get_vertex_layout_bytes(vertex_data.layout()) * vertex_data.vertex_count();

    MVE_VAL_ASSERT(buffer_size != 0, "[Renderer] Attempt to allocate empty vertex buffer")

    Buffer staging_buffer = create_buffer(
        m_vma_allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* data;
    vmaMapMemory(m_vma_allocator, staging_buffer.vma_allocation, &data);
    memcpy(data, vertex_data.data_ptr(), buffer_size);
    vmaUnmapMemory(m_vma_allocator, staging_buffer.vma_allocation);

    Buffer buffer = create_buffer(
        m_vma_allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

    defer_to_command_buffer_front([this, staging_buffer, buffer, buffer_size](vk::CommandBuffer command_buffer) {
        cmd_copy_buffer(m_vk_loader, command_buffer, staging_buffer.vk_handle, buffer.vk_handle, buffer_size);

        auto barrier
            = vk::BufferMemoryBarrier()
                  .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                  .setDstAccessMask(vk::AccessFlagBits::eVertexAttributeRead)
                  .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                  .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                  .setBuffer(buffer.vk_handle)
                  .setOffset(0)
                  .setSize(buffer_size);

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eVertexInput,
            {},
            0,
            nullptr,
            1,
            &barrier,
            0,
            nullptr,
            m_vk_loader);

        defer_to_next_frame([this, staging_buffer](uint32_t) {
            vmaDestroyBuffer(
                m_vma_allocator, static_cast<VkBuffer>(staging_buffer.vk_handle), staging_buffer.vma_allocation);
        });
    });

    std::optional<size_t> id;
    for (size_t i = 0; i < m_vertex_buffers_new.size(); i++) {
        if (!m_vertex_buffers_new[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = m_vertex_buffers_new.size();
        m_vertex_buffers_new.emplace_back();
    }
    m_vertex_buffers_new[*id] = { buffer, vertex_data.vertex_count() };

    log().debug("[Renderer] Vertex buffer created with ID: {}", *id);

    return { *this, *id };
}

void Renderer::bind_vertex_buffer(const VertexBuffer& vertex_buffer)
{
    vk::DeviceSize offset = 0;
    m_current_draw_state.command_buffer.bindVertexBuffers(
        0, 1, &m_vertex_buffers_new[vertex_buffer.m_handle]->buffer.vk_handle, &offset, m_vk_loader);
}

IndexBuffer Renderer::create_index_buffer(const std::vector<uint32_t>& indices)
{
    size_t buffer_size = sizeof(uint32_t) * indices.size();

    Buffer staging_buffer = create_buffer(
        m_vma_allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* data;
    vmaMapMemory(m_vma_allocator, staging_buffer.vma_allocation, &data);
    memcpy(data, indices.data(), buffer_size);
    vmaUnmapMemory(m_vma_allocator, staging_buffer.vma_allocation);

    Buffer buffer = create_buffer(
        m_vma_allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        {});

    defer_to_command_buffer_front([this, staging_buffer, buffer, buffer_size](vk::CommandBuffer command_buffer) {
        cmd_copy_buffer(m_vk_loader, command_buffer, staging_buffer.vk_handle, buffer.vk_handle, buffer_size);

        auto barrier
            = vk::BufferMemoryBarrier()
                  .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                  .setDstAccessMask(vk::AccessFlagBits::eVertexAttributeRead)
                  .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                  .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                  .setBuffer(buffer.vk_handle)
                  .setOffset(0)
                  .setSize(buffer_size);

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eVertexInput,
            {},
            0,
            nullptr,
            1,
            &barrier,
            0,
            nullptr,
            m_vk_loader);

        defer_to_next_frame([this, staging_buffer](uint32_t) {
            vmaDestroyBuffer(
                m_vma_allocator, static_cast<VkBuffer>(staging_buffer.vk_handle), staging_buffer.vma_allocation);
        });
    });

    std::optional<size_t> id;
    for (size_t i = 0; i < m_index_buffers_new.size(); i++) {
        if (!m_index_buffers_new[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = m_index_buffers_new.size();
        m_index_buffers_new.emplace_back();
    }
    m_index_buffers_new[*id] = { buffer, indices.size() };

    log().debug("[Renderer] Index buffer created with ID: {}", *id);

    return { *this, *id };
}

void Renderer::draw_index_buffer(const IndexBuffer& index_buffer)
{
    IndexBufferImpl& index_buffer_impl = *m_index_buffers_new[index_buffer.m_handle];
    m_current_draw_state.command_buffer.bindIndexBuffer(
        index_buffer_impl.buffer.vk_handle, 0, vk::IndexType::eUint32, m_vk_loader);
    m_current_draw_state.command_buffer.drawIndexed(index_buffer_impl.index_count, 1, 0, 0, 0, m_vk_loader);
}

GraphicsPipeline Renderer::create_graphics_pipeline(
    const Shader& vertex_shader, const Shader& fragment_shader, const VertexLayout& vertex_layout, bool depth_test)
{
    size_t layout = create_graphics_pipeline_layout(m_vk_loader, vertex_shader, fragment_shader);

    vk::Pipeline vk_pipeline = create_vk_graphics_pipeline(
        m_vk_loader,
        m_vk_device,
        vertex_shader,
        fragment_shader,
        m_graphics_pipeline_layouts_new.at(layout)->vk_handle,
        m_vk_render_pass,
        vertex_layout,
        m_msaa_samples,
        depth_test);

    std::optional<size_t> id;
    for (size_t i = 0; i < m_graphics_pipelines_new.size(); i++) {
        if (!m_graphics_pipelines_new[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = m_graphics_pipelines_new.size();
        m_graphics_pipelines_new.emplace_back();
    }
    m_graphics_pipelines_new[*id] = { .layout = layout, .pipeline = vk_pipeline };

    log().debug("[Renderer] Graphics pipeline created with ID: {}", *id);

    return { *this, *id };
}

DescriptorSet Renderer::create_descriptor_set(
    const GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set)
{
    std::vector<DescriptorSetImpl> descriptor_sets;
    descriptor_sets.reserve(c_frames_in_flight);

    vk::DescriptorSetLayout layout = m_descriptor_set_layouts.at(
        m_graphics_pipeline_layouts_new.at(m_graphics_pipelines_new.at(graphics_pipeline.handle())->layout)
            ->descriptor_set_layouts.at(descriptor_set.set()));

    for (int i = 0; i < c_frames_in_flight; i++) {
        descriptor_sets.push_back(m_descriptor_set_allocator.create(m_vk_loader, m_vk_device, layout));
    }

    FrameInFlight& ref_frame = m_frames_in_flight.at(0);
    std::optional<size_t> id;
    for (size_t i = 0; i < ref_frame.descriptor_sets.size(); i++) {
        if (!ref_frame.descriptor_sets[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = ref_frame.descriptor_sets.size();
        for (FrameInFlight& frame : m_frames_in_flight) {
            frame.descriptor_sets.emplace_back();
        }
    }
    int i = 0;
    for (FrameInFlight& frame : m_frames_in_flight) {
        frame.descriptor_sets[*id] = descriptor_sets.at(i);
        i++;
    }

    log().debug("[Renderer] Descriptor set created with ID: {}", *id);

    return { *this, *id };
}

void Renderer::bind_graphics_pipeline(const GraphicsPipeline& graphics_pipeline)
{
    if (m_current_draw_state.current_pipeline == graphics_pipeline.handle()) {
        return;
    }
    m_current_draw_state.command_buffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        m_graphics_pipelines_new.at(graphics_pipeline.handle())->pipeline,
        m_vk_loader);
    m_current_draw_state.current_pipeline = graphics_pipeline.handle();
}

void Renderer::write_descriptor_binding(
    const DescriptorSet& descriptor_set,
    const ShaderDescriptorBinding& descriptor_binding,
    const UniformBuffer& uniform_buffer)
{
    DeferredDescriptorWriteData write_data {
        .counter = c_frames_in_flight,
        .data_type = DescriptorBindingType::uniform_buffer,
        .data_handle = uniform_buffer.m_handle,
        .descriptor_handle = descriptor_set.m_handle,
        .binding = descriptor_binding.binding()
    };
    m_deferred_descriptor_writes.push_back(write_data);
}

void Renderer::bind_descriptor_set(DescriptorSet& descriptor_set)
{
    bind_descriptor_sets(1, { &descriptor_set, nullptr, nullptr, nullptr });
}

UniformBuffer Renderer::create_uniform_buffer(const ShaderDescriptorBinding& descriptor_binding)
{
    MVE_VAL_ASSERT(
        descriptor_binding.type() == ShaderDescriptorType::uniform_buffer,
        "[Renderer] Failed to create uniform buffer as binding is not of type uniform buffer")

    uint32_t struct_size = descriptor_binding.block().size();

    FrameInFlight& ref_frame = m_frames_in_flight.at(0);
    std::optional<size_t> id;
    for (size_t i = 0; i < ref_frame.uniform_buffers.size(); i++) {
        if (!ref_frame.uniform_buffers[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = ref_frame.uniform_buffers.size();
        for (FrameInFlight& frame : m_frames_in_flight) {
            frame.uniform_buffers.emplace_back();
        }
    }
    int i = 0;
    for (FrameInFlight& frame : m_frames_in_flight) {
        Buffer buffer = create_buffer(
            m_vma_allocator,
            struct_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO,
            VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        void* ptr;
        vmaMapMemory(m_vma_allocator, buffer.vma_allocation, &ptr);
        frame.uniform_buffers[*id] = { buffer, struct_size, static_cast<std::byte*>(ptr) };
        i++;
    }

    log().debug("[Renderer] Uniform buffer created with ID: {}", *id);

    return { *this, *id };
}

void Renderer::update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, float value, bool persist)
{
    update_uniform<float>(uniform_buffer, location, value, persist);
}
void Renderer::update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, mve::Vector2 value, bool persist)
{
    update_uniform<mve::Vector2>(uniform_buffer, location, value, persist);
}
void Renderer::update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, mve::Vector3 value, bool persist)
{
    update_uniform<mve::Vector3>(uniform_buffer, location, value, persist);
}
void Renderer::update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, mve::Vector4 value, bool persist)
{
    update_uniform<mve::Vector4>(uniform_buffer, location, value, persist);
}
void Renderer::update_uniform(
    UniformBuffer& uniform_buffer, UniformLocation location, const mve::Matrix3& value, bool persist)
{
    update_uniform<mve::Matrix3>(uniform_buffer, location, value, persist);
}
void Renderer::update_uniform(
    UniformBuffer& uniform_buffer, UniformLocation location, const mve::Matrix4& value, bool persist)
{
    update_uniform<mve::Matrix4>(uniform_buffer, location, value, persist);
}

void Renderer::destroy(Texture& texture)
{
    MVE_VAL_ASSERT(texture.is_valid(), "[Renderer] Attempted to destroy invalid texture")
    log().debug("[Renderer] Destroyed texture with ID: {}", texture.handle());
    uint64_t handle = texture.handle();
    texture.invalidate();
    defer_after_all_frames([this, handle](uint32_t) {
        TextureImpl& texture = m_textures.at(handle);
        m_vk_device.destroy(texture.vk_sampler, nullptr, m_vk_loader);
        m_vk_device.destroy(texture.vk_image_view, nullptr, m_vk_loader);
        vmaDestroyImage(m_vma_allocator, static_cast<VkImage>(texture.image.vk_handle), texture.image.vma_allocation);
        m_textures.erase(handle);
    });
}

// TODO: mip-mapping
Texture Renderer::create_texture(TextureFormat format, uint32_t width, uint32_t height, const std::byte* data)
{
    MVE_VAL_ASSERT(width != 0 && height != 0, "[Renderer] Attempt to create texture with 0 width or height")
    uint32_t mip_levels = 1;

    vk::Format vk_format;
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

    Buffer staging_buffer = create_buffer(
        m_vma_allocator,
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* data_ptr;
    vmaMapMemory(m_vma_allocator, staging_buffer.vma_allocation, &data_ptr);
    memcpy(data_ptr, data, static_cast<size_t>(size));
    vmaUnmapMemory(m_vma_allocator, staging_buffer.vma_allocation);

    Image image = create_image(
        m_vma_allocator,
        width,
        height,
        mip_levels,
        vk::SampleCountFlagBits::e1,
        vk_format,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        false);

    defer_to_command_buffer_front([this, image, mip_levels, staging_buffer, width, height, vk_format](
                                      vk::CommandBuffer command_buffer) {
        cmd_transition_image_layout(
            m_vk_loader,
            command_buffer,
            image.vk_handle,
            vk_format,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal,
            mip_levels);

        cmd_copy_buffer_to_image(m_vk_loader, command_buffer, staging_buffer.vk_handle, image.vk_handle, width, height);

        cmd_generate_mipmaps(
            m_vk_loader, m_vk_physical_device, command_buffer, image.vk_handle, vk_format, width, height, mip_levels);

        defer_to_next_frame([this, staging_buffer](uint32_t) {
            vmaDestroyBuffer(
                m_vma_allocator, static_cast<VkBuffer>(staging_buffer.vk_handle), staging_buffer.vma_allocation);
        });
    });

    vk::ImageView image_view = create_image_view(
        m_vk_loader, m_vk_device, image.vk_handle, vk_format, vk::ImageAspectFlagBits::eColor, mip_levels);

    vk::Sampler sampler = create_texture_sampler(m_vk_loader, m_vk_physical_device, m_vk_device, mip_levels);

    TextureImpl texture {
        .image = image, .vk_image_view = image_view, .vk_sampler = sampler, .mip_levels = mip_levels
    };

    auto handle = m_resource_handle_count;
    m_resource_handle_count++;
    m_textures.insert({ handle, texture });

    log().debug("[Renderer] Texture created with ID: {}", handle);

    return { *this, handle };
}

Texture Renderer::create_texture(const std::filesystem::path& path)
{
    int width;
    int height;
    int channels;
    std::string path_string = path.string();
    stbi_uc* pixels = stbi_load(path_string.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    vk::DeviceSize size = width * height * 4;
    MVE_ASSERT(pixels != nullptr, "[Renderer] Failed to load texture image")

    return create_texture(
        TextureFormat::rgba,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        reinterpret_cast<const std::byte*>(pixels));
}

Texture Renderer::create_texture(Image image, vk::ImageView image_view, vk::Sampler sampler, uint32_t mip_levels)
{
    TextureImpl texture {
        .image = image, .vk_image_view = image_view, .vk_sampler = sampler, .mip_levels = mip_levels
    };

    auto handle = m_resource_handle_count;
    m_resource_handle_count++;
    m_textures.insert({ handle, texture });

    log().debug("[Renderer] Texture created with ID: {}", handle);

    return { *this, handle };
}

void Renderer::draw_vertex_buffer(const VertexBuffer& vertex_buffer)
{
    VertexBufferImpl& vertex_buffer_impl = *m_vertex_buffers_new.at(vertex_buffer.handle());
    m_current_draw_state.command_buffer.bindVertexBuffers(0, vertex_buffer_impl.buffer.vk_handle, { 0 });
    m_current_draw_state.command_buffer.draw(vertex_buffer_impl.vertex_count, 1, 0, 0);
}

void Renderer::destroy(DescriptorSet& descriptor_set)
{
    MVE_VAL_ASSERT(descriptor_set.is_valid(), "[Renderer] Attempted to destroy invalid descriptor set")
    log().debug("[Renderer] Destroyed descriptor set with ID: {}", descriptor_set.handle());
    uint64_t handle = descriptor_set.handle();
    descriptor_set.invalidate();
    defer_after_all_frames([this, handle](uint32_t) {
        std::vector<DescriptorSetImpl> sets_to_delete;
        std::transform(
            m_frames_in_flight.cbegin(),
            m_frames_in_flight.cend(),
            std::back_inserter(sets_to_delete),
            [&](const FrameInFlight& frame) { return *frame.descriptor_sets.at(handle); });
        for (FrameInFlight& frame : m_frames_in_flight) {
            frame.descriptor_sets[handle].reset();
        }
        for (DescriptorSetImpl set : sets_to_delete) {
            m_descriptor_set_allocator.free(m_vk_loader, m_vk_device, set);
        }
    });
}

void Renderer::destroy(GraphicsPipeline& graphics_pipeline)
{
    MVE_VAL_ASSERT(graphics_pipeline.is_valid(), "[Renderer] Attempted to destroy invalid graphics pipeline")
    log().debug("[Renderer] Destroyed graphics pipeline with ID: {}", graphics_pipeline.handle());
    size_t handle = graphics_pipeline.handle();
    graphics_pipeline.invalidate();
    defer_after_all_frames([this, handle](uint32_t) {
        // Descriptor set layouts
        std::vector<DescriptorSetLayoutHandleImpl> deleted_descriptor_set_layout_handles;
        for (auto& [set, set_layout] :
             m_graphics_pipeline_layouts_new.at(m_graphics_pipelines_new.at(handle)->layout)->descriptor_set_layouts) {
            m_vk_device.destroy(m_descriptor_set_layouts.at(set_layout), nullptr, m_vk_loader);
            deleted_descriptor_set_layout_handles.push_back(set_layout);
        }
        for (DescriptorSetLayoutHandleImpl descriptor_set_layout_handle : deleted_descriptor_set_layout_handles) {
            m_descriptor_set_layouts.erase(descriptor_set_layout_handle);
        }

        // Pipeline layout
        m_vk_device.destroy(
            m_graphics_pipeline_layouts_new.at(m_graphics_pipelines_new.at(handle)->layout)->vk_handle,
            nullptr,
            m_vk_loader);
        m_graphics_pipeline_layouts_new[m_graphics_pipelines_new.at(handle)->layout].reset();

        // Graphics pipeline
        m_vk_device.destroy(m_graphics_pipelines_new.at(handle)->pipeline, nullptr, m_vk_loader);
        m_graphics_pipelines_new[handle].reset();
    });
}

void Renderer::destroy(UniformBuffer& uniform_buffer)
{
    MVE_VAL_ASSERT(uniform_buffer.is_valid(), "[Renderer] Attempted to destroy invalid uniform buffer")
    log().debug("[Renderer] Destroyed uniform buffer with ID: {}", uniform_buffer.handle());
    uniform_buffer.invalidate();
    size_t handle = uniform_buffer.handle();
    defer_after_all_frames([this, handle](uint32_t) {
        for (const FrameInFlight& frame : m_frames_in_flight) {
            UniformBufferImpl uniform_buffer = *(frame.uniform_buffers.at(handle));
            vmaUnmapMemory(m_vma_allocator, uniform_buffer.buffer.vma_allocation);
            vmaDestroyBuffer(
                m_vma_allocator,
                static_cast<VkBuffer>(uniform_buffer.buffer.vk_handle),
                uniform_buffer.buffer.vma_allocation);
        }
        for (FrameInFlight& frame : m_frames_in_flight) {
            frame.uniform_buffers[handle].reset();
        }
    });
}

void Renderer::destroy(IndexBuffer& index_buffer)
{
    MVE_VAL_ASSERT(index_buffer.is_valid(), "[Renderer] Attempted to destroy invalid index buffer")
    log().debug("[Renderer] Destroyed index buffer with ID: {}", index_buffer.handle());
    uint64_t handle = index_buffer.handle();
    index_buffer.invalidate();
    defer_after_all_frames([this, handle](uint32_t) {
        vmaDestroyBuffer(
            m_vma_allocator,
            static_cast<VkBuffer>(m_index_buffers_new.at(handle)->buffer.vk_handle),
            m_index_buffers_new.at(handle)->buffer.vma_allocation);
        m_index_buffers_new[handle].reset();
    });
}

void Renderer::bind_descriptor_sets(const DescriptorSet& descriptor_set_a, const DescriptorSet& descriptor_set_b)
{
    bind_descriptor_sets(2, { &descriptor_set_a, &descriptor_set_b, nullptr, nullptr });
}

void Renderer::bind_descriptor_sets(uint32_t num, const std::array<const DescriptorSet*, 4>& descriptor_sets)
{
    std::array<vk::DescriptorSet, 4> sets;
    for (uint32_t i = 0; i < num; i++) {
        sets[i] = (m_frames_in_flight[m_current_draw_state.frame_index]
                       .descriptor_sets[descriptor_sets[i]->m_handle]
                       ->vk_handle);
    }

    m_current_draw_state.command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        m_graphics_pipeline_layouts_new[m_graphics_pipelines_new[m_current_draw_state.current_pipeline]->layout]
            ->vk_handle,
        0,
        num,
        sets.data(),
        0,
        nullptr,
        m_vk_loader);
}

void Renderer::end_render_pass_present()
{
    m_current_draw_state.command_buffer.endRenderPass(m_vk_loader);
}
Framebuffer Renderer::create_framebuffer(std::function<void(void)> callback)
{
    std::optional<size_t> id;
    for (size_t i = 0; i < m_framebuffers.size(); i++) {
        if (!m_framebuffers[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = m_framebuffers.size();
        m_framebuffers.emplace_back();
    }
    m_framebuffers[*id] = std::move(create_framebuffer_impl(m_vk_loader, callback));

    log().debug("[Renderer] Framebuffer created with ID: {}", *id);

    return { *this, *id };
}
void Renderer::destroy(Framebuffer& framebuffer)
{
    MVE_VAL_ASSERT(framebuffer.is_valid(), "[Renderer] Attempted to destroy invalid framebuffer")
    log().debug("[Renderer] Destroyed framebuffer with ID: {}", framebuffer.handle());
    size_t handle = framebuffer.handle();
    framebuffer.invalidate();
    defer_after_all_frames([this, handle](uint32_t) {
        FramebufferImpl& framebuffer = m_framebuffers.at(handle).value();

        m_textures.erase(handle);

        for (vk::Framebuffer& buffer : framebuffer.vk_framebuffers) {
            m_vk_device.destroy(buffer, nullptr, m_vk_loader);
        }

        m_framebuffers.at(handle).reset();
    });
}
void Renderer::recreate_framebuffers()
{
    std::vector<std::pair<size_t, std::optional<std::function<void(void)>>>> ids_to_recreate;
    for (size_t i = 0; i < m_framebuffers.size(); i++) {
        if (m_framebuffers[i].has_value()) {
            ids_to_recreate.emplace_back(i, m_framebuffers[i]->callback);
            for (vk::Framebuffer& buffer : m_framebuffers[i]->vk_framebuffers) {
                m_vk_device.destroy(buffer, nullptr, m_vk_loader);
            }
        }
    }
    for (auto& [id, callback] : ids_to_recreate) {
        m_framebuffers[id] = std::move(create_framebuffer_impl(m_vk_loader, callback));
    }
    for (const std::optional<FramebufferImpl>& framebuffer : m_framebuffers) {
        std::invoke(*framebuffer->callback);
    }
}
Renderer::FramebufferImpl Renderer::create_framebuffer_impl(
    const vk::DispatchLoaderDynamic& loader, std::optional<std::function<void(void)>> callback)
{
    RenderImage render_image = create_color_image(
        m_vk_loader,
        m_vk_device,
        m_vma_allocator,
        m_vk_swapchain_extent,
        m_vk_swapchain_image_format.format,
        vk::SampleCountFlagBits::e1);

    std::vector<vk::Framebuffer> framebuffers;
    framebuffers.reserve(m_vk_swapchain_framebuffers.size());

    for (size_t i = 0; i < m_vk_swapchain_framebuffers.size(); i++) {

        std::array<vk::ImageView, 3> attachments {};
        if (m_msaa_samples == vk::SampleCountFlagBits::e1) {
            attachments = { render_image.vk_image_view, m_depth_image.vk_image_view };
        }
        else {
            attachments = { m_color_image.vk_image_view, m_depth_image.vk_image_view, render_image.vk_image_view };
        }

        auto framebuffer_info
            = vk::FramebufferCreateInfo()
                  .setRenderPass(m_vk_render_pass)
                  .setAttachmentCount(m_msaa_samples == vk::SampleCountFlagBits::e1 ? 2 : 3)
                  .setPAttachments(attachments.data())
                  .setWidth(m_vk_swapchain_extent.width)
                  .setHeight(m_vk_swapchain_extent.height)
                  .setLayers(1);

        vk::ResultValue<vk::Framebuffer> framebuffer_result
            = m_vk_device.createFramebuffer(framebuffer_info, nullptr, loader);
        MVE_ASSERT(framebuffer_result.result == vk::Result::eSuccess, "[Renderer] Failed to create framebuffer")
        framebuffers.push_back(framebuffer_result.value);
    }

    vk::Sampler sampler = create_texture_sampler(m_vk_loader, m_vk_physical_device, m_vk_device, 1);

    Texture texture = create_texture(render_image.image, render_image.vk_image_view, sampler, 1);

    FramebufferImpl framebuffer_impl {
        .vk_framebuffers = std::move(framebuffers),
        .texture = std::move(texture),
        .callback = std::move(callback),
        .size = { static_cast<int>(m_vk_swapchain_extent.width), static_cast<int>(m_vk_swapchain_extent.height) }
    };

    return framebuffer_impl;
}

void Renderer::end_render_pass_framebuffer(const Framebuffer& framebuffer)
{
    m_current_draw_state.command_buffer.endRenderPass(m_vk_loader);
    //
    //    auto subresource_range
    //        = vk::ImageSubresourceRange()
    //              .setAspectMask(vk::ImageAspectFlagBits::eColor)
    //              .setBaseArrayLayer(0)
    //              .setLayerCount(1)
    //              .setLevelCount(1);
    //
    //    auto barrier
    //        = vk::ImageMemoryBarrier()
    //              .setImage(m_textures[framebuffer.texture().m_handle].image.vk_handle)
    //              .setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
    //              .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
    //              .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
    //              .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
    //              .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
    //              .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
    //              .setSubresourceRange(subresource_range);
    //
    //    m_current_draw_state.command_buffer.pipelineBarrier(
    //        vk::PipelineStageFlagBits::eColorAttachmentOutput,
    //        vk::PipelineStageFlagBits::eFragmentShader,
    //        {},
    //        0,
    //        nullptr,
    //        0,
    //        nullptr,
    //        1,
    //        &barrier);
}
const Texture& Renderer::framebuffer_texture(const Framebuffer& framebuffer)
{
    return m_framebuffers[framebuffer.m_handle]->texture;
}
Vector2i Renderer::framebuffer_size(const Framebuffer& framebuffer)
{
    return m_framebuffers[framebuffer.m_handle]->size;
}
std::string Renderer::gpu_name() const
{
    return m_vk_physical_device.getProperties(m_vk_loader).deviceName;
}
mve::Vector2i Renderer::texture_size(const Texture& texture) const
{
    MVE_VAL_ASSERT(texture.m_valid, "[Renderer] Attempt to get size on invalid texture")
    const TextureImpl& texture_impl = m_textures.at(texture.m_handle);
    return { static_cast<int>(texture_impl.image.width), static_cast<int>(texture_impl.image.height) };
}

Renderer::DescriptorSetAllocator::DescriptorSetAllocator()
    : m_sizes({ { vk::DescriptorType::eSampler, 0.5f },
                { vk::DescriptorType::eCombinedImageSampler, 4.0f },
                { vk::DescriptorType::eSampledImage, 4.0f },
                { vk::DescriptorType::eStorageImage, 1.0f },
                { vk::DescriptorType::eUniformTexelBuffer, 1.0f },
                { vk::DescriptorType::eStorageTexelBuffer, 1.0f },
                { vk::DescriptorType::eUniformBuffer, 2.0f },
                { vk::DescriptorType::eStorageBuffer, 2.0f },
                { vk::DescriptorType::eUniformBufferDynamic, 1.0f },
                { vk::DescriptorType::eStorageBufferDynamic, 1.0f },
                { vk::DescriptorType::eInputAttachment, 0.5f } })
    , m_max_sets_per_pool(1000)
    , m_current_pool_index(0)
    , m_id_count(0)
{
}

vk::DescriptorPool Renderer::DescriptorSetAllocator::create_pool(
    const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::DescriptorPoolCreateFlags flags)
{
    std::vector<vk::DescriptorPoolSize> sizes;
    sizes.reserve(m_sizes.size());

    std::transform(
        m_sizes.cbegin(), m_sizes.cend(), std::back_inserter(sizes), [&](std::pair<vk::DescriptorType, float> s) {
            return vk::DescriptorPoolSize(
                s.first, static_cast<uint32_t>(s.second * static_cast<float>(m_max_sets_per_pool)));
        });

    auto pool_info = vk::DescriptorPoolCreateInfo()
                         .setFlags(flags)
                         .setMaxSets(static_cast<uint32_t>(m_max_sets_per_pool))
                         .setPoolSizes(sizes);

    vk::ResultValue<vk::DescriptorPool> descriptor_pool_result
        = device.createDescriptorPool(pool_info, nullptr, loader);
    MVE_ASSERT(descriptor_pool_result.result == vk::Result::eSuccess, "[Renderer] Failed to create descriptor pool")
    return descriptor_pool_result.value;
}

void Renderer::DescriptorSetAllocator::cleanup(const vk::DispatchLoaderDynamic& loader, vk::Device device)
{
    for (std::optional<DescriptorSetImpl>& set : m_descriptor_sets) {
        if (set.has_value()) {
            device.freeDescriptorSets(set->vk_pool, 1, &(set->vk_handle), loader);
        }
    }
    for (vk::DescriptorPool descriptor_pool : m_descriptor_pools) {
        device.destroy(descriptor_pool, nullptr, loader);
    }
}

Renderer::DescriptorSetImpl Renderer::DescriptorSetAllocator::create(
    const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::DescriptorSetLayout layout)
{
    if (m_descriptor_pools.empty()) {
        m_descriptor_pools.push_back(create_pool(loader, device, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet));
        m_current_pool_index = 0;
    }

    std::optional<vk::DescriptorSet> descriptor_set
        = try_create(loader, m_descriptor_pools.at(m_current_pool_index), device, layout);

    if (!descriptor_set.has_value()) {
        for (size_t i = 0; i < m_descriptor_pools.size(); i++) {
            if (i == m_current_pool_index) {
                continue;
            }
            descriptor_set = try_create(loader, m_descriptor_pools.at(i), device, layout);
            if (descriptor_set.has_value()) {
                m_current_pool_index = i;
                break;
            }
        }

        if (!descriptor_set.has_value()) {
            m_descriptor_pools.push_back(
                create_pool(loader, device, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet));
            m_current_pool_index = m_descriptor_pools.size() - 1;
            descriptor_set = try_create(loader, m_descriptor_pools.at(m_current_pool_index), device, layout);

            MVE_ASSERT(descriptor_set.has_value(), "[Renderer] Failed to allocate descriptor set")
        }
    }
    std::optional<size_t> id;
    for (size_t i = 0; i < m_descriptor_sets.size(); i++) {
        if (!m_descriptor_sets[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = m_descriptor_sets.size();
        m_descriptor_sets.emplace_back();
    }
    DescriptorSetImpl descriptor_set_impl {
        .id = *id, .vk_handle = *descriptor_set, .vk_pool = m_descriptor_pools.at(m_current_pool_index)
    };
    m_descriptor_sets[*id] = descriptor_set_impl;
    return descriptor_set_impl;
}

std::optional<vk::DescriptorSet> Renderer::DescriptorSetAllocator::try_create(
    const vk::DispatchLoaderDynamic& loader, vk::DescriptorPool pool, vk::Device device, vk::DescriptorSetLayout layout)
{
    auto alloc_info
        = vk::DescriptorSetAllocateInfo().setDescriptorPool(pool).setDescriptorSetCount(1).setPSetLayouts(&layout);

    vk::ResultValue<std::vector<vk::DescriptorSet>> descriptor_sets_result
        = device.allocateDescriptorSets(alloc_info, loader);

    if (descriptor_sets_result.result == vk::Result::eErrorOutOfPoolMemory
        || descriptor_sets_result.result == vk::Result::eErrorFragmentedPool) {
        return {};
    }
    else if (descriptor_sets_result.result == vk::Result::eSuccess) {
        return descriptor_sets_result.value.at(0);
    }
    else {
        MVE_ASSERT(false, "[Renderer] Failed to allocate descriptor sets")
    }
}

void Renderer::DescriptorSetAllocator::free(
    const vk::DispatchLoaderDynamic& loader, vk::Device device, DescriptorSetImpl descriptor_set)
{
    device.freeDescriptorSets(descriptor_set.vk_pool, 1, &descriptor_set.vk_handle, loader);
    m_descriptor_sets[descriptor_set.id].reset();
}
}