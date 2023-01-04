#include "renderer.hpp"

#include <fstream>
#include <set>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "logger.hpp"
#include "shader.hpp"
#include "window.hpp"

namespace mve {
Renderer::Renderer(
    const Window& window,
    const std::string& app_name,
    int app_version_major,
    int app_version_minor,
    int app_version_patch)
    : c_frames_in_flight(2)
    , m_resource_handle_count(0)
    , m_deferred_function_id_count(0)
{
    m_vk_instance = create_vk_instance(app_name, app_version_major, app_version_minor, app_version_patch);
#ifdef MVE_ENABLE_VALIDATION_LAYERS
    m_vk_debug_utils_messenger = create_vk_debug_messenger(m_vk_instance);
#endif
    m_vk_surface = create_vk_surface(m_vk_instance, window.glfw_handle());
    m_vk_physical_device = pick_vk_physical_device(m_vk_instance, m_vk_surface);
    m_vk_queue_family_indices = get_vk_queue_family_indices(m_vk_physical_device, m_vk_surface);
    m_vk_device = create_vk_logical_device(m_vk_physical_device, m_vk_queue_family_indices);

    SwapchainSupportDetails swapchain_support_details
        = get_vk_swapchain_support_details(m_vk_physical_device, m_vk_surface);

    m_vk_swapchain_image_format = choose_vk_swapchain_surface_format(swapchain_support_details.formats);
    m_vk_swapchain_extent = get_vk_swapchain_extent(swapchain_support_details.capabilities, window.glfw_handle());
    m_vk_swapchain = create_vk_swapchain(
        m_vk_physical_device,
        m_vk_device,
        m_vk_surface,
        m_vk_swapchain_image_format,
        m_vk_swapchain_extent,
        m_vk_queue_family_indices);

    m_vk_swapchain_images = get_vk_swapchain_images(m_vk_device, m_vk_swapchain);

    m_vk_swapchain_image_views
        = create_vk_swapchain_image_views(m_vk_device, m_vk_swapchain_images, m_vk_swapchain_image_format.format);

    m_vk_graphics_queue = m_vk_device.getQueue(m_vk_queue_family_indices.graphics_family.value(), 0);
    m_vk_present_queue = m_vk_device.getQueue(m_vk_queue_family_indices.present_family.value(), 0);

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = m_vk_physical_device;
    allocatorCreateInfo.device = m_vk_device;
    allocatorCreateInfo.instance = m_vk_instance;

    vmaCreateAllocator(&allocatorCreateInfo, &m_vma_allocator);

    m_vk_command_pool = create_vk_command_pool(m_vk_device, m_vk_queue_family_indices);

    m_depth_image = create_depth_image(
        m_vk_physical_device,
        m_vk_device,
        m_vk_command_pool,
        m_vk_graphics_queue,
        m_vma_allocator,
        m_vk_swapchain_extent);

    m_vk_render_pass = create_vk_render_pass(
        m_vk_device, m_vk_swapchain_image_format.format, find_depth_format(m_vk_physical_device));

    m_vk_swapchain_framebuffers = create_vk_framebuffers(
        m_vk_device, m_vk_swapchain_image_views, m_vk_render_pass, m_vk_swapchain_extent, m_depth_image.vk_image_view);

    m_frames_in_flight = create_frames_in_flight(m_vk_device, m_vk_command_pool, c_frames_in_flight);

    m_current_draw_state.is_drawing = false;
    m_current_draw_state.frame_index = 0;
    m_current_draw_state.image_index = 0;
    m_current_draw_state.command_buffer = VK_NULL_HANDLE;
}

bool Renderer::has_validation_layer_support()
{
    vk::ResultValue<std::vector<vk::LayerProperties>> available_layers_result = vk::enumerateInstanceLayerProperties();
    if (available_layers_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to get validation layer properties.");
    }

    std::vector<vk::LayerProperties> available_layers = std::move(available_layers_result.value);

    const std::vector<const char*> validation_layers = get_vk_validation_layer_exts();

    for (const std::string& validation_layer : validation_layers) {
        bool layer_found = false;
        for (vk::LayerProperties available_layer : available_layers) {
            if (validation_layer == available_layer.layerName) {
                layer_found = true;
                break;
            }
        }
        if (!layer_found) {
            return false;
        }
    }
    return true;
}

vk::Instance Renderer::create_vk_instance(
    const std::string& app_name, int app_version_major, int app_version_minor, int app_version_patch)
{
#ifdef MVE_ENABLE_VALIDATION_LAYERS
    if (!has_validation_layer_support()) {
        throw std::runtime_error("Validation layers requested but not available");
    }
#endif

    auto application_info
        = vk::ApplicationInfo()
              .setPApplicationName(app_name.c_str())
              .setApplicationVersion(VK_MAKE_VERSION(app_version_major, app_version_minor, app_version_patch))
              .setPEngineName("Mini Vulkan Engine")
              .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
              .setApiVersion(VK_API_VERSION_1_1);

    std::vector<const char*> exts = get_vk_instance_required_exts();

#ifdef MVE_ENABLE_VALIDATION_LAYERS

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

    vk::ResultValue<vk::Instance> instance_result = vk::createInstance(instance_create_info);
    if (instance_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create instance");
    }
    return instance_result.value;
}

std::vector<const char*> Renderer::get_vk_validation_layer_exts()
{
    return std::vector<const char*> { "VK_LAYER_KHRONOS_validation" };
}

vk::PhysicalDevice Renderer::pick_vk_physical_device(vk::Instance instance, vk::SurfaceKHR surface)
{
    vk::ResultValue<std::vector<vk::PhysicalDevice>> physical_devices_result = instance.enumeratePhysicalDevices();
    if (physical_devices_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to get physical devices (GPUs)");
    }

    std::vector<vk::PhysicalDevice> physical_devices = std::move(physical_devices_result.value);

    if (physical_devices.empty()) {
        throw std::runtime_error("Failed to find Vulkan device");
    }

    for (vk::PhysicalDevice physical_device : physical_devices) {
        LOG->debug("Vulkan Device Found: {0}", physical_device.getProperties().deviceName);
    }

    for (vk::PhysicalDevice physical_device : physical_devices) {
        if (is_vk_physical_device_suitable(physical_device, surface)) {
            LOG->info("Using Vulkan Device: {0}", physical_device.getProperties().deviceName);
            return physical_device;
        }
    }

    throw std::runtime_error("Failed to find a suitable Vulkan device");
}

bool Renderer::is_vk_physical_device_suitable(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
    QueueFamilyIndices indices = get_vk_queue_family_indices(physical_device, surface);

    vk::ResultValue<std::vector<vk::ExtensionProperties>> available_exts_result
        = physical_device.enumerateDeviceExtensionProperties();
    if (available_exts_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to get device extension properties.");
    }

    std::vector<vk::ExtensionProperties> available_exts = std::move(available_exts_result.value);

    std::vector<const char*> required_exts = get_vk_device_required_exts();

    for (const std::string& required_ext : required_exts) {
        bool is_available = false;
        for (const vk::ExtensionProperties& ext_props : available_exts) {
            if (required_ext == ext_props.extensionName) {
                is_available = true;
                break;
            }
        }
        if (!is_available) {
            return false;
        }
    }

    SwapchainSupportDetails swapchain_support_details = get_vk_swapchain_support_details(physical_device, surface);
    bool is_swapchain_adequate
        = !swapchain_support_details.formats.empty() && !swapchain_support_details.present_modes.empty();

    vk::PhysicalDeviceFeatures supported_features = physical_device.getFeatures();

    return indices.is_complete() && is_swapchain_adequate && supported_features.samplerAnisotropy;
}

Renderer::QueueFamilyIndices Renderer::get_vk_queue_family_indices(
    vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queue_families = physical_device.getQueueFamilyProperties();
    int i = 0;
    for (const vk::QueueFamilyProperties& queue_family : queue_families) {
        if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics_family = i;
        }

        vk::ResultValue<unsigned int> surface_support_result = physical_device.getSurfaceSupportKHR(i, surface);
        if (surface_support_result.result != vk::Result::eSuccess) {
            throw std::runtime_error("[Renderer] Failed to get GPU surface support.");
        }

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
    vk::PhysicalDevice physical_device, QueueFamilyIndices queue_family_indices)
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

    std::vector<const char*> required_exts = get_vk_device_required_exts();

#ifdef MVE_ENABLE_VALIDATION_LAYERS
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

    vk::ResultValue<vk::Device> device_result = physical_device.createDevice(device_create_info);
    if (device_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create device.");
    }
    return device_result.value;
}
vk::SurfaceKHR Renderer::create_vk_surface(vk::Instance instance, GLFWwindow* window)
{
    VkSurfaceKHR surface;
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
    return { surface };
}

std::vector<const char*> Renderer::get_vk_device_required_exts()
{
    return std::vector<const char*> { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}

Renderer::SwapchainSupportDetails Renderer::get_vk_swapchain_support_details(
    vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
    SwapchainSupportDetails details;

    vk::ResultValue<vk::SurfaceCapabilitiesKHR> capabilities_result
        = physical_device.getSurfaceCapabilitiesKHR(surface);
    if (capabilities_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to get surface capabilities.");
    }
    details.capabilities = std::move(capabilities_result.value);

    vk::ResultValue<std::vector<vk::SurfaceFormatKHR>> formats_result = physical_device.getSurfaceFormatsKHR(surface);
    if (formats_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to get surface formats.");
    }
    details.formats = std::move(formats_result.value);

    vk::ResultValue<std::vector<vk::PresentModeKHR>> present_modes_result
        = physical_device.getSurfacePresentModesKHR(surface);
    if (present_modes_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to get surface present modes.");
    }
    details.present_modes = std::move(present_modes_result.value);

    return details;
}
vk::SurfaceFormatKHR Renderer::choose_vk_swapchain_surface_format(
    const std::vector<vk::SurfaceFormatKHR>& available_formats)
{
    for (const vk::SurfaceFormatKHR& available_format : available_formats) {
        if (available_format.format == vk::Format::eB8G8R8A8Srgb
            && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return available_format;
        }
    }
    return available_formats[0];
}
vk::PresentModeKHR Renderer::choose_vk_swapchain_present_mode(
    const std::vector<vk::PresentModeKHR>& available_present_modes)
{
    for (const vk::PresentModeKHR& available_present_mode : available_present_modes) {
        if (available_present_mode == vk::PresentModeKHR::eMailbox) {
            return available_present_mode;
        }
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
    vk::PhysicalDevice physical_device,
    vk::Device device,
    vk::SurfaceKHR surface,
    vk::SurfaceFormatKHR surface_format,
    vk::Extent2D surface_extent,
    QueueFamilyIndices queue_family_indices)
{
    SwapchainSupportDetails swapchain_support_details = get_vk_swapchain_support_details(physical_device, surface);

    vk::PresentModeKHR present_mode = choose_vk_swapchain_present_mode(swapchain_support_details.present_modes);

    uint32_t image_count = swapchain_support_details.capabilities.minImageCount + 1;
    if (swapchain_support_details.capabilities.maxImageCount > 0
        && image_count > swapchain_support_details.capabilities.maxImageCount) {
        image_count = swapchain_support_details.capabilities.maxImageCount;
    }

    assert(queue_family_indices.is_complete());

    uint32_t indices_arr[]
        = { queue_family_indices.graphics_family.value(), queue_family_indices.present_family.value() };

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
        swapchain_create_info.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndexCount(2)
            .setPQueueFamilyIndices(indices_arr);
    }
    else {
        swapchain_create_info.setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount(0)
            .setPQueueFamilyIndices(nullptr);
    }

    vk::ResultValue<vk::SwapchainKHR> swapchain_result = device.createSwapchainKHR(swapchain_create_info);
    if (swapchain_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create swapchain.");
    }
    return swapchain_result.value;
}

std::vector<vk::Image> Renderer::get_vk_swapchain_images(vk::Device device, vk::SwapchainKHR swapchain)
{
    vk::ResultValue<std::vector<vk::Image>> images_result = device.getSwapchainImagesKHR(swapchain);
    if (images_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to get swapchain images.");
    }
    return images_result.value;
}

std::vector<vk::ImageView> Renderer::create_vk_swapchain_image_views(
    vk::Device device, const std::vector<vk::Image>& swapchain_images, vk::Format image_format)
{
    std::vector<vk::ImageView> image_views;

    for (vk::Image swapchain_image : swapchain_images) {
        image_views.push_back(
            create_image_view(device, swapchain_image, image_format, vk::ImageAspectFlagBits::eColor));
    }

    return image_views;
}

vk::Pipeline Renderer::create_vk_graphics_pipeline(
    vk::Device device,
    const Shader& vertex_shader,
    const Shader& fragment_shader,
    vk::PipelineLayout pipeline_layout,
    vk::RenderPass render_pass,
    const VertexLayout& vertex_layout)
{
    std::vector<uint32_t> vertex_spv_code = vertex_shader.spv_code();
    auto vertex_shader_create_info
        = vk::ShaderModuleCreateInfo().setCodeSize(vertex_spv_code.size() * 4).setPCode(vertex_spv_code.data());

    vk::ResultValue<vk::ShaderModule> vertex_shader_module_result
        = device.createShaderModule(vertex_shader_create_info);
    if (vertex_shader_module_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create vertex shader module.");
    }
    vk::ShaderModule vertex_shader_module = std::move(vertex_shader_module_result.value);

    std::vector<uint32_t> fragment_spv_code = fragment_shader.spv_code();
    auto fragment_shader_create_info
        = vk::ShaderModuleCreateInfo().setCodeSize(fragment_spv_code.size() * 4).setPCode(fragment_spv_code.data());

    vk::ResultValue<vk::ShaderModule> fragment_shader_module_result
        = device.createShaderModule(fragment_shader_create_info);
    if (fragment_shader_module_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create fragment shader module.");
    }
    vk::ShaderModule fragment_shader_module = std::move(fragment_shader_module_result.value);

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
              .setSampleShadingEnable(false)
              .setRasterizationSamples(vk::SampleCountFlagBits::e1)
              .setMinSampleShading(1.0f)
              .setPSampleMask(nullptr)
              .setAlphaToCoverageEnable(false)
              .setAlphaToOneEnable(false);

    auto depth_stencil
        = vk::PipelineDepthStencilStateCreateInfo()
              .setDepthTestEnable(VK_TRUE)
              .setDepthWriteEnable(VK_TRUE)
              .setDepthCompareOp(vk::CompareOp::eLess)
              .setDepthBoundsTestEnable(VK_FALSE)
              .setStencilTestEnable(VK_FALSE);

    auto color_blend_attachment
        = vk::PipelineColorBlendAttachmentState()
              .setColorWriteMask(
                  vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB
                  | vk::ColorComponentFlagBits::eA)
              .setBlendEnable(false)
              .setSrcColorBlendFactor(vk::BlendFactor::eOne)
              .setDstColorBlendFactor(vk::BlendFactor::eZero)
              .setColorBlendOp(vk::BlendOp::eAdd)
              .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
              .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
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

    vk::ResultValue<vk::Pipeline> pipeline_result = device.createGraphicsPipeline(nullptr, graphics_pipeline_info);
    if (pipeline_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
    vk::Pipeline graphics_pipeline = pipeline_result.value;

    device.destroy(vertex_shader_module);
    device.destroy(fragment_shader_module);

    return graphics_pipeline;
}

vk::PipelineLayout Renderer::create_vk_pipeline_layout(const std::vector<DescriptorSetLayoutHandle>& layouts)
{
    std::vector<vk::DescriptorSetLayout> vk_layouts;
    for (DescriptorSetLayoutHandle handle : layouts) {
        vk_layouts.push_back(m_descriptor_set_layouts.at(handle));
    }

    auto pipeline_layout_info
        = vk::PipelineLayoutCreateInfo()
              .setSetLayouts(vk_layouts)
              .setPushConstantRangeCount(0)
              .setPPushConstantRanges(nullptr);

    vk::ResultValue<vk::PipelineLayout> pipeline_layout_result = m_vk_device.createPipelineLayout(pipeline_layout_info);
    if (pipeline_layout_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create pipline layout.");
    }
    return pipeline_layout_result.value;
}

vk::RenderPass Renderer::create_vk_render_pass(vk::Device device, vk::Format swapchain_format, vk::Format depth_format)
{
    auto color_attachment
        = vk::AttachmentDescription()
              .setFormat(swapchain_format)
              .setSamples(vk::SampleCountFlagBits::e1)
              .setLoadOp(vk::AttachmentLoadOp::eClear)
              .setStoreOp(vk::AttachmentStoreOp::eStore)
              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
              .setInitialLayout(vk::ImageLayout::eUndefined)
              .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    auto depth_attachment
        = vk::AttachmentDescription()
              .setFormat(depth_format)
              .setSamples(vk::SampleCountFlagBits::e1)
              .setLoadOp(vk::AttachmentLoadOp::eClear)
              .setStoreOp(vk::AttachmentStoreOp::eDontCare)
              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
              .setInitialLayout(vk::ImageLayout::eUndefined)
              .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    auto color_attachment_ref
        = vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    auto depth_attachment_ref
        = vk::AttachmentReference().setAttachment(1).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    auto subpass = vk::SubpassDescription()
                       .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                       .setColorAttachmentCount(1)
                       .setPColorAttachments(&color_attachment_ref)
                       .setPDepthStencilAttachment(&depth_attachment_ref);

    auto subpass_dependency
        = vk::SubpassDependency()
              .setSrcSubpass(VK_SUBPASS_EXTERNAL)
              .setDstSubpass(0)
              .setSrcStageMask(
                  vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
              .setSrcAccessMask(vk::AccessFlagBits::eNone)
              .setDstStageMask(
                  vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
              .setDstAccessMask(
                  vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    std::array<vk::AttachmentDescription, 2> attachments = { color_attachment, depth_attachment };

    auto render_pass_info
        = vk::RenderPassCreateInfo()
              .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
              .setPAttachments(attachments.data())
              .setSubpassCount(1)
              .setPSubpasses(&subpass)
              .setSubpassCount(1)
              .setPDependencies(&subpass_dependency);

    vk::ResultValue<vk::RenderPass> render_pass_result = device.createRenderPass(render_pass_info);
    if (render_pass_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create render pass.");
    }
    return render_pass_result.value;
}

std::vector<vk::Framebuffer> Renderer::create_vk_framebuffers(
    vk::Device device,
    const std::vector<vk::ImageView>& swapchain_image_views,
    vk::RenderPass render_pass,
    vk::Extent2D swapchain_extent,
    vk::ImageView depth_image_view)
{
    std::vector<vk::Framebuffer> framebuffers;

    for (const vk::ImageView& swapchain_image_view : swapchain_image_views) {

        std::array<vk::ImageView, 2> attachments = { swapchain_image_view, depth_image_view };

        auto framebuffer_info
            = vk::FramebufferCreateInfo()
                  .setRenderPass(render_pass)
                  .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
                  .setPAttachments(attachments.data())
                  .setWidth(swapchain_extent.width)
                  .setHeight(swapchain_extent.height)
                  .setLayers(1);

        vk::ResultValue<vk::Framebuffer> framebuffer_result = device.createFramebuffer(framebuffer_info);
        if (framebuffer_result.result != vk::Result::eSuccess) {
            throw std::runtime_error("[Renderer] Failed to create framebuffer.");
        }
        framebuffers.push_back(std::move(framebuffer_result.value));
    }
    return framebuffers;
}

vk::CommandPool Renderer::create_vk_command_pool(vk::Device device, QueueFamilyIndices queue_family_indices)
{
    assert(queue_family_indices.is_complete());

    auto command_pool_info = vk::CommandPoolCreateInfo()
                                 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                                 .setQueueFamilyIndex(queue_family_indices.graphics_family.value());

    vk::ResultValue<vk::CommandPool> command_pool_result = device.createCommandPool(command_pool_info);
    if (command_pool_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create command buffer.");
    }
    return command_pool_result.value;
}

std::vector<vk::CommandBuffer> Renderer::create_vk_command_buffers(
    vk::Device device, vk::CommandPool command_pool, int frames_in_flight)
{
    auto buffer_alloc_info
        = vk::CommandBufferAllocateInfo()
              .setCommandPool(command_pool)
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandBufferCount(static_cast<uint32_t>(frames_in_flight));

    vk::ResultValue<std::vector<vk::CommandBuffer>> command_buffers_result
        = device.allocateCommandBuffers(buffer_alloc_info);
    if (command_buffers_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to allocate command buffers.");
    }
    return command_buffers_result.value;
}

Renderer::~Renderer()
{
#ifdef MVE_ENABLE_VALIDATION_LAYERS
    cleanup_vk_debug_messenger();
#endif
    vk::Result wait_result = m_vk_device.waitIdle();

    cleanup_vk_swapchain();

    for (auto& [handle, texture] : m_textures) {
        m_vk_device.destroy(texture.vk_sampler);
        m_vk_device.destroy(texture.vk_image_view);
        vmaDestroyImage(m_vma_allocator, texture.image.vk_handle, texture.image.vma_allocation);
    }

    m_descriptor_set_allocator.cleanup(m_vk_device);

    for (FrameInFlight& frame : m_frames_in_flight) {
        for (auto& pair : frame.uniform_buffers) {
            vmaUnmapMemory(m_vma_allocator, pair.second.buffer.vma_allocation);
            vmaDestroyBuffer(m_vma_allocator, pair.second.buffer.vk_handle, pair.second.buffer.vma_allocation);
        }
    }

    for (auto& [handle, layout] : m_descriptor_set_layouts) {
        m_vk_device.destroy(layout);
    }

    for (auto& buffer : m_vertex_buffers) {
        vmaDestroyBuffer(m_vma_allocator, buffer.second.buffer.vk_handle, buffer.second.buffer.vma_allocation);
    }

    for (auto& buffer : m_index_buffers) {
        vmaDestroyBuffer(m_vma_allocator, buffer.second.buffer.vk_handle, buffer.second.buffer.vma_allocation);
    }

    vmaDestroyAllocator(m_vma_allocator);

    for (auto& [handle, pipeline] : m_graphics_pipelines) {
        m_vk_device.destroy(pipeline);
    }

    for (auto& [handle, layout] : m_graphics_pipeline_layouts) {
        m_vk_device.destroy(layout);
    }

    m_vk_device.destroy(m_vk_render_pass);

    for (FrameInFlight& frame : m_frames_in_flight) {
        m_vk_device.destroy(frame.render_finished_semaphore);
        m_vk_device.destroy(frame.image_available_semaphore);
        m_vk_device.destroy(frame.in_flight_fence);
    }

    m_vk_device.destroy(m_vk_command_pool);

    m_vk_device.destroy();

    m_vk_instance.destroy(m_vk_surface);
    m_vk_instance.destroy();
}

void Renderer::recreate_swapchain(const Window& window)
{
    glm::ivec2 window_size;
    glfwGetFramebufferSize(window.glfw_handle(), &(window_size.x), &(window_size.y));

    while (window_size == glm::ivec2(0, 0)) {
        glfwGetFramebufferSize(window.glfw_handle(), &(window_size.x), &(window_size.y));
        window.wait_for_events();
    }

    vk::Result wait_result = m_vk_device.waitIdle();
    if (wait_result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to wait idle for swapchain recreation.");
    }

    cleanup_vk_swapchain();

    SwapchainSupportDetails swapchain_support_details
        = get_vk_swapchain_support_details(m_vk_physical_device, m_vk_surface);

    m_vk_swapchain_extent = get_vk_swapchain_extent(swapchain_support_details.capabilities, window.glfw_handle());

    m_vk_swapchain = create_vk_swapchain(
        m_vk_physical_device,
        m_vk_device,
        m_vk_surface,
        m_vk_swapchain_image_format,
        m_vk_swapchain_extent,
        m_vk_queue_family_indices);

    m_vk_swapchain_images = get_vk_swapchain_images(m_vk_device, m_vk_swapchain);

    m_vk_swapchain_image_views
        = create_vk_swapchain_image_views(m_vk_device, m_vk_swapchain_images, m_vk_swapchain_image_format.format);

    m_depth_image = create_depth_image(
        m_vk_physical_device,
        m_vk_device,
        m_vk_command_pool,
        m_vk_graphics_queue,
        m_vma_allocator,
        m_vk_swapchain_extent);

    m_vk_swapchain_framebuffers = create_vk_framebuffers(
        m_vk_device, m_vk_swapchain_image_views, m_vk_render_pass, m_vk_swapchain_extent, m_depth_image.vk_image_view);
}

void Renderer::cleanup_vk_swapchain()
{
    m_vk_device.destroy(m_depth_image.vk_image_view);
    vmaDestroyImage(m_vma_allocator, m_depth_image.image.vk_handle, m_depth_image.image.vma_allocation);

    for (vk::Framebuffer framebuffer : m_vk_swapchain_framebuffers) {
        m_vk_device.destroy(framebuffer);
    }
    for (vk::ImageView image_view : m_vk_swapchain_image_views) {
        m_vk_device.destroy(image_view);
    }
    m_vk_device.destroy(m_vk_swapchain);
}

std::vector<const char*> Renderer::get_vk_instance_required_exts()
{
    uint32_t glfw_ext_count = 0;
    const char** glfw_exts;
    glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

    std::vector<const char*> exts(glfw_exts, glfw_exts + glfw_ext_count);

#ifdef MVE_ENABLE_VALIDATION_LAYERS
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

    if (func != nullptr) {
        VkDebugUtilsMessengerEXT debug_messenger;
        func(instance, &debug_create_info, nullptr, &debug_messenger);
        return { debug_messenger };
    }
    else {
        throw std::runtime_error("Failed to create Vulkan debug messenger");
    }
}

VkBool32 Renderer::vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
    VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data_ptr)
{
    if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LOG->warn("[Vulkan Debug] " + std::string(callback_data->pMessage));
    }

    return false;
}

void Renderer::cleanup_vk_debug_messenger()
{
    auto func
        = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vk_instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(m_vk_instance, m_vk_debug_utils_messenger, nullptr);
    }
}

Renderer::VertexBuffer Renderer::create_vertex_buffer(
    vk::Device device,
    vk::CommandPool command_pool,
    vk::Queue graphics_queue,
    VmaAllocator allocator,
    const VertexData& vertex_data)
{
    size_t buffer_size = get_vertex_layout_bytes(vertex_data.layout()) * vertex_data.vertex_count();

    Buffer staging_buffer = create_buffer(
        allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* data;
    vmaMapMemory(allocator, staging_buffer.vma_allocation, &data);
    memcpy(data, vertex_data.data_ptr(), buffer_size);
    vmaUnmapMemory(allocator, staging_buffer.vma_allocation);

    Buffer vertex_buffer = create_buffer(
        allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

    copy_buffer(device, command_pool, graphics_queue, staging_buffer.vk_handle, vertex_buffer.vk_handle, buffer_size);

    vmaDestroyBuffer(allocator, staging_buffer.vk_handle, staging_buffer.vma_allocation);

    return { vertex_buffer, vertex_data.vertex_count() };
}

VertexBufferHandle Renderer::create_vertex_buffer(const VertexData& vertex_data)
{
    VertexBuffer vertex_buffer
        = create_vertex_buffer(m_vk_device, m_vk_command_pool, m_vk_graphics_queue, m_vma_allocator, vertex_data);
    m_vertex_buffers[VertexBufferHandle(m_resource_handle_count)] = vertex_buffer;
    m_resource_handle_count++;

    return VertexBufferHandle(m_resource_handle_count - 1);
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
            offset += sizeof(glm::vec2);
            break;
        case VertexAttributeType::vec3:
            description.setFormat(vk::Format::eR32G32B32Sfloat);
            offset += sizeof(glm::vec3);
            break;
        case VertexAttributeType::vec4:
            description.setFormat(vk::Format::eR32G32B32A32Sfloat);
            offset += sizeof(glm::vec4);
            break;
        }

        attribute_descriptions.push_back(description);
    }

    return attribute_descriptions;
}

std::vector<Renderer::FrameInFlight> Renderer::create_frames_in_flight(
    vk::Device device, vk::CommandPool command_pool, int frame_count)
{
    auto frames_in_flight = std::vector<FrameInFlight>();
    frames_in_flight.reserve(frame_count);

    std::vector<vk::CommandBuffer> command_buffers = create_vk_command_buffers(device, command_pool, frame_count);

    auto semaphore_info = vk::SemaphoreCreateInfo();
    auto fence_info = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (int i = 0; i < frame_count; i++) {
        FrameInFlight frame;
        vk::ResultValue<vk::Semaphore> image_available_semaphore_result = device.createSemaphore(semaphore_info);
        if (image_available_semaphore_result.result != vk::Result::eSuccess) {
            throw std::runtime_error("[Renderer] Failed to create image available semaphore.");
        }
        frame.image_available_semaphore = std::move(image_available_semaphore_result.value);

        vk::ResultValue<vk::Semaphore> render_finished_semaphore_result = device.createSemaphore(semaphore_info);
        if (render_finished_semaphore_result.result != vk::Result::eSuccess) {
            throw std::runtime_error("[Renderer] Failed to create render finished semaphore.");
        }
        frame.render_finished_semaphore = std::move(render_finished_semaphore_result.value);

        vk::ResultValue<vk::Fence> in_flight_fence_result = device.createFence(fence_info);
        if (in_flight_fence_result.result != vk::Result::eSuccess) {
            throw std::runtime_error("[Renderer] Failed to create in flight fence.");
        }
        frame.in_flight_fence = std::move(in_flight_fence_result.value);
        frame.command_buffer = command_buffers.at(i);
        frame.uniform_buffers = {};
        frames_in_flight.push_back(frame);
    }

    return frames_in_flight;
}

void Renderer::queue_destroy(VertexBufferHandle handle)
{
    push_wait_for_frames([this, handle](uint32_t) {
        vmaDestroyBuffer(
            m_vma_allocator,
            m_vertex_buffers.at(handle).buffer.vk_handle,
            m_vertex_buffers.at(handle).buffer.vma_allocation);
        m_vertex_buffers.erase(handle);
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
    vma_alloc_info.flags = flags;

    VkBuffer vk_buffer;
    VmaAllocation allocation;

    vmaCreateBuffer(allocator, &buffer_info, &vma_alloc_info, &vk_buffer, &allocation, nullptr);

    return { vk_buffer, allocation };
}

void Renderer::copy_buffer(
    vk::Device device,
    vk::CommandPool command_pool,
    vk::Queue graphics_queue,
    vk::Buffer src_buffer,
    vk::Buffer dst_buffer,
    vk::DeviceSize size)
{
    vk::CommandBuffer command_buffer = begin_single_submit(device, command_pool);

    auto copy_region = vk::BufferCopy().setSrcOffset(0).setDstOffset(0).setSize(size);
    command_buffer.copyBuffer(src_buffer, dst_buffer, 1, &copy_region);

    end_single_submit(device, command_pool, command_buffer, graphics_queue);
}

Renderer::IndexBuffer Renderer::create_index_buffer(
    vk::Device device,
    vk::CommandPool command_pool,
    vk::Queue graphics_queue,
    VmaAllocator allocator,
    const std::vector<uint32_t>& index_data)
{
    size_t buffer_size = sizeof(uint32_t) * index_data.size();

    Buffer staging_buffer = create_buffer(
        allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* data;
    vmaMapMemory(allocator, staging_buffer.vma_allocation, &data);
    memcpy(data, index_data.data(), buffer_size);
    vmaUnmapMemory(allocator, staging_buffer.vma_allocation);

    Buffer vertex_buffer = create_buffer(
        allocator,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

    copy_buffer(device, command_pool, graphics_queue, staging_buffer.vk_handle, vertex_buffer.vk_handle, buffer_size);

    vmaDestroyBuffer(allocator, staging_buffer.vk_handle, staging_buffer.vma_allocation);

    return { vertex_buffer, index_data.size() };
}

IndexBufferHandle Renderer::create_index_buffer(const std::vector<uint32_t>& index_data)
{
    IndexBuffer index_buffer
        = create_index_buffer(m_vk_device, m_vk_command_pool, m_vk_graphics_queue, m_vma_allocator, index_data);
    m_index_buffers[IndexBufferHandle(m_resource_handle_count)] = index_buffer;
    m_resource_handle_count++;

    return IndexBufferHandle(m_resource_handle_count - 1);
}

void Renderer::queue_destroy(IndexBufferHandle handle)
{
    push_wait_for_frames([this, handle](uint32_t) {
        vmaDestroyBuffer(
            m_vma_allocator,
            m_index_buffers.at(handle).buffer.vk_handle,
            m_index_buffers.at(handle).buffer.vma_allocation);
        m_index_buffers.erase(handle);
    });
}

void Renderer::begin(const Window& window)
{
    if (m_current_draw_state.is_drawing) {
        throw std::runtime_error("[Renderer] Already drawing.");
    }

    m_current_draw_state.is_drawing = true;

    FrameInFlight& frame = m_frames_in_flight[m_current_draw_state.frame_index];

    wait_ready();

    vk::ResultValue<uint32_t> acquire_result
        = m_vk_device.acquireNextImageKHR(m_vk_swapchain, UINT64_MAX, frame.image_available_semaphore, nullptr);
    if (acquire_result.result != vk::Result::eSuccess && acquire_result.result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }
    else if (acquire_result.result == vk::Result::eSuboptimalKHR) {
        recreate_swapchain(window);
        m_current_draw_state.is_drawing = false;
        return;
    }
    m_current_draw_state.image_index = acquire_result.value;

    m_vk_device.resetFences({ frame.in_flight_fence });

    frame.command_buffer.reset();

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

    m_current_draw_state.command_buffer = m_frames_in_flight[m_current_draw_state.frame_index].command_buffer;

    auto buffer_begin_info = vk::CommandBufferBeginInfo();
    vk::Result begin_result = m_current_draw_state.command_buffer.begin(buffer_begin_info);
    if (begin_result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to begin command buffer recording.");
    }

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

    m_current_draw_state.command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

    auto viewport
        = vk::Viewport()
              .setX(0.0f)
              .setY(0.0f)
              .setWidth(static_cast<float>(m_vk_swapchain_extent.width))
              .setHeight(static_cast<float>(m_vk_swapchain_extent.height))
              .setMinDepth(0.0f)
              .setMaxDepth(1.0f);

    m_current_draw_state.command_buffer.setViewport(0, { viewport });

    auto scissor = vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_vk_swapchain_extent);

    m_current_draw_state.command_buffer.setScissor(0, { scissor });
}

void Renderer::end(const Window& window)
{
    m_current_draw_state.command_buffer.endRenderPass();

    vk::Result end_result = m_current_draw_state.command_buffer.end();
    if (end_result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to end command buffer recording.");
    }

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

    vk::Result graphics_submit_result = m_vk_graphics_queue.submit({ submit_info }, frame.in_flight_fence);
    if (graphics_submit_result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to submit to graphics queue.");
    }

    vk::SwapchainKHR swapchains[] = { m_vk_swapchain };

    auto present_info
        = vk::PresentInfoKHR()
              .setWaitSemaphores(signal_semaphores)
              .setSwapchains(swapchains)
              .setPImageIndices(&(m_current_draw_state.image_index));

    vk::Result present_result = m_vk_present_queue.presentKHR(present_info);
    if (present_result == vk::Result::eSuboptimalKHR || present_result == vk::Result::eErrorOutOfDateKHR) {
        recreate_swapchain(window);
    }
    else if (present_result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to present frame");
    }

    m_current_draw_state.frame_index = (m_current_draw_state.frame_index + 1) % c_frames_in_flight;

    m_current_draw_state.is_drawing = false;
}

void Renderer::draw_vertex_buffer(VertexBufferHandle handle)
{
    VertexBuffer& vertex_buffer = m_vertex_buffers.at(handle);
    m_current_draw_state.command_buffer.bindVertexBuffers(0, vertex_buffer.buffer.vk_handle, { 0 });
    m_current_draw_state.command_buffer.draw(vertex_buffer.vertex_count, 1, 0, 0);
}

void Renderer::bind_vertex_buffer(VertexBufferHandle handle)
{
    m_current_draw_state.command_buffer.bindVertexBuffers(0, m_vertex_buffers.at(handle).buffer.vk_handle, { 0 });
}

void Renderer::draw_index_buffer(IndexBufferHandle handle)
{
    IndexBuffer& index_buffer = m_index_buffers.at(handle);
    m_current_draw_state.command_buffer.bindIndexBuffer(index_buffer.buffer.vk_handle, 0, vk::IndexType::eUint32);
    m_current_draw_state.command_buffer.drawIndexed(index_buffer.index_count, 1, 0, 0, 0);
}

vk::DescriptorSetLayout Renderer::create_vk_descriptor_set_layout(vk::Device device)
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
        = device.createDescriptorSetLayout(layout_info);
    if (descriptor_set_layout_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create descriptor set layout");
    }
    return descriptor_set_layout_result.value;
}

vk::DescriptorPool Renderer::create_vk_descriptor_pool(vk::Device device, int frames_in_flight)
{
    auto pool_size = vk::DescriptorPoolSize()
                         .setType(vk::DescriptorType::eUniformBuffer)
                         .setDescriptorCount(static_cast<uint32_t>(frames_in_flight));

    auto pool_info = vk::DescriptorPoolCreateInfo()
                         .setPoolSizeCount(1)
                         .setPPoolSizes(&pool_size)
                         .setMaxSets(static_cast<uint32_t>(frames_in_flight));

    vk::ResultValue<vk::DescriptorPool> descriptor_pool_result = device.createDescriptorPool(pool_info);
    if (descriptor_pool_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create descriptor pool.");
    }
    return descriptor_pool_result.value;
}

std::vector<vk::DescriptorSet> Renderer::create_vk_descriptor_sets(
    vk::Device device, vk::DescriptorSetLayout layout, vk::DescriptorPool pool, int count)
{
    auto layouts = std::vector<vk::DescriptorSetLayout>(count, layout);

    auto alloc_info = vk::DescriptorSetAllocateInfo()
                          .setDescriptorPool(pool)
                          .setDescriptorSetCount(static_cast<uint32_t>(count))
                          .setPSetLayouts(layouts.data());

    vk::ResultValue<std::vector<vk::DescriptorSet>> descriptor_sets_result = device.allocateDescriptorSets(alloc_info);
    if (descriptor_sets_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to allocate descriptor sets.");
    }
    return descriptor_sets_result.value;
}

bool Renderer::is_valid(VertexBufferHandle handle)
{
    return m_vertex_buffers.contains(handle);
}

bool Renderer::is_valid(IndexBufferHandle handle)
{
    return m_index_buffers.contains(handle);
}

UniformBufferHandle Renderer::create_uniform_buffer(
    const UniformStructLayout& struct_layout, DescriptorSetHandle descriptor_set, uint32_t binding)
{
    auto handle = UniformBufferHandle(m_resource_handle_count);
    m_resource_handle_count++;

    for (FrameInFlight& frame : m_frames_in_flight) {

        vk::DeviceSize buffer_size = struct_layout.size_bytes();

        Buffer buffer = create_buffer(
            m_vma_allocator, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        void* ptr;
        vmaMapMemory(m_vma_allocator, buffer.vma_allocation, &ptr);

        frame.uniform_buffers[handle] = UniformBuffer { buffer, static_cast<std::byte*>(ptr) };
    }

    size_t layout_bytes = struct_layout.size_bytes();

    push_to_all_frames([this, handle, layout_bytes, descriptor_set, binding](uint32_t frame_index) {
        auto buffer_info
            = vk::DescriptorBufferInfo()
                  .setBuffer(this->m_frames_in_flight.at(frame_index).uniform_buffers.at(handle).buffer.vk_handle)
                  .setOffset(0)
                  .setRange(layout_bytes);

        auto descriptor_write
            = vk::WriteDescriptorSet()
                  .setDstSet(this->m_frames_in_flight.at(frame_index).descriptor_sets.at(descriptor_set))
                  .setDstBinding(binding)
                  .setDstArrayElement(0)
                  .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                  .setDescriptorCount(1)
                  .setPBufferInfo(&buffer_info);

        this->m_vk_device.updateDescriptorSets(1, &descriptor_write, 0, nullptr);
    });

    return handle;
}

void Renderer::update_uniform(UniformBufferHandle handle, UniformLocation location, glm::mat4 value, bool persist)
{
    auto func = [this, handle, location, value](uint32_t frame_index) {
        this->update_uniform(handle, location, (void*)(&value), sizeof(glm::mat4), frame_index);
    };
    if (persist) {
        push_to_all_frames(func);
    }
    else {
        push_to_next_frame(func);
    }
}

void Renderer::bind_descriptor_set(DescriptorSetHandle handle, GraphicsPipelineLayoutHandle pipeline_layout)
{
    m_current_draw_state.command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        m_graphics_pipeline_layouts.at(pipeline_layout),
        0,
        1,
        &(m_frames_in_flight.at(m_current_draw_state.frame_index).descriptor_sets.at(handle)),
        0,
        nullptr);
}

glm::ivec2 Renderer::extent() const
{
    return { m_vk_swapchain_extent.width, m_vk_swapchain_extent.height };
}

void Renderer::wait_ready()
{
    FrameInFlight& frame = m_frames_in_flight[m_current_draw_state.frame_index];

    vk::Result fence_wait_result = m_vk_device.waitForFences(frame.in_flight_fence, true, UINT64_MAX);
    if (fence_wait_result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed waiting for frame (fences)");
    }
}

void Renderer::update_uniform(UniformBufferHandle handle, UniformLocation location, float value, bool persist)
{
    auto func = [this, handle, location, value](uint32_t frame_index) {
        this->update_uniform(handle, location, (void*)(&value), sizeof(float), frame_index);
    };
    if (persist) {
        push_to_all_frames(func);
    }
    else {
        push_to_next_frame(func);
    }
}

void Renderer::update_uniform(
    UniformBufferHandle handle, UniformLocation location, void* data_ptr, size_t size, uint32_t frame_index)
{
    UniformBuffer& buffer = m_frames_in_flight.at(frame_index).uniform_buffers.at(handle);
    memcpy(&(buffer.mapped_ptr[location.value_of()]), data_ptr, size);
}

void Renderer::update_uniform(UniformBufferHandle handle, UniformLocation location, glm::vec2 value, bool persist)
{
    auto func = [this, handle, location, value](uint32_t frame_index) {
        this->update_uniform(handle, location, (void*)(&value), sizeof(decltype(value)), frame_index);
    };
    if (persist) {
        push_to_all_frames(func);
    }
    else {
        push_to_next_frame(func);
    }
}

void Renderer::update_uniform(UniformBufferHandle handle, UniformLocation location, glm::vec3 value, bool persist)
{
    auto func = [this, handle, location, value](uint32_t frame_index) {
        this->update_uniform(handle, location, (void*)(&value), sizeof(decltype(value)), frame_index);
    };
    if (persist) {
        push_to_all_frames(func);
    }
    else {
        push_to_next_frame(func);
    }
}

void Renderer::update_uniform(UniformBufferHandle handle, UniformLocation location, glm::vec4 value, bool persist)
{
    auto func = [this, handle, location, value](uint32_t frame_index) {
        this->update_uniform(handle, location, (void*)(&value), sizeof(decltype(value)), frame_index);
    };
    if (persist) {
        push_to_all_frames(func);
    }
    else {
        push_to_next_frame(func);
    }
}

void Renderer::update_uniform(UniformBufferHandle handle, UniformLocation location, glm::mat2 value, bool persist)
{
    auto func = [this, handle, location, value](uint32_t frame_index) {
        this->update_uniform(handle, location, (void*)(&value), sizeof(decltype(value)), frame_index);
    };
    if (persist) {
        push_to_all_frames(func);
    }
    else {
        push_to_next_frame(func);
    }
}

void Renderer::update_uniform(UniformBufferHandle handle, UniformLocation location, glm::mat3 value, bool persist)
{
    auto func = [this, handle, location, value](uint32_t frame_index) {
        this->update_uniform(handle, location, (void*)(&value), sizeof(decltype(value)), frame_index);
    };
    if (persist) {
        push_to_all_frames(func);
    }
    else {
        push_to_next_frame(func);
    }
}

DescriptorSetLayoutHandle Renderer::create_descriptor_set_layout(const std::vector<DescriptorType>& layout)
{
    if (layout.empty()) {
        throw std::runtime_error("[Renderer] Descriptor set layout is empty");
    }

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (uint32_t i = 0; i < layout.size(); i++) {
        auto binding
            = vk::DescriptorSetLayoutBinding().setBinding(i).setDescriptorCount(1).setPImmutableSamplers(nullptr);

        switch (layout.at(i)) {
        case e_uniform_buffer:
            binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
            binding.setStageFlags(vk::ShaderStageFlagBits::eAll);
            break;
        case e_combined_image_sampler:
            binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
            binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
            break;
        }
        bindings.push_back(binding);
    }

    auto layout_info = vk::DescriptorSetLayoutCreateInfo().setBindings(bindings);

    vk::ResultValue<vk::DescriptorSetLayout> descriptor_set_layout_result
        = m_vk_device.createDescriptorSetLayout(layout_info);
    if (descriptor_set_layout_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create descriptor set layout.");
    }
    vk::DescriptorSetLayout vk_layout = std::move(descriptor_set_layout_result.value);

    auto handle = DescriptorSetLayoutHandle(m_resource_handle_count);
    m_descriptor_set_layouts.insert({ handle, vk_layout });
    m_resource_handle_count++;

    return handle;
}

DescriptorSetHandle Renderer::create_descriptor_set(DescriptorSetLayoutHandle layout)
{
    std::vector<vk::DescriptorSet> descriptor_sets;
    descriptor_sets.reserve(c_frames_in_flight);

    for (int i = 0; i < c_frames_in_flight; i++) {
        descriptor_sets.push_back(m_descriptor_set_allocator.create(m_vk_device, m_descriptor_set_layouts.at(layout)));
    }

    auto handle = DescriptorSetHandle(m_resource_handle_count);
    m_resource_handle_count++;

    int i = 0;
    for (FrameInFlight& frame : m_frames_in_flight) {
        frame.descriptor_sets.insert({ handle, descriptor_sets.at(i) });
        i++;
    }

    return handle;
}

GraphicsPipelineHandle Renderer::create_graphics_pipeline(
    GraphicsPipelineLayoutHandle layout,
    const Shader& vertex_shader,
    const Shader& fragment_shader,
    const VertexLayout& vertex_layout)
{
    vk::Pipeline vk_pipeline = create_vk_graphics_pipeline(
        m_vk_device,
        vertex_shader,
        fragment_shader,
        m_graphics_pipeline_layouts.at(layout),
        m_vk_render_pass,
        vertex_layout);

    auto handle = GraphicsPipelineHandle(m_resource_handle_count);
    m_resource_handle_count++;
    m_graphics_pipelines.insert({ handle, vk_pipeline });

    return handle;
}

GraphicsPipelineLayoutHandle Renderer::create_graphics_pipeline_layout(
    const std::vector<DescriptorSetLayoutHandle>& layouts)
{
    vk::PipelineLayout vk_layout = create_vk_pipeline_layout(layouts);

    auto handle = GraphicsPipelineLayoutHandle(m_resource_handle_count);
    m_resource_handle_count++;
    m_graphics_pipeline_layouts.insert({ handle, vk_layout });

    return handle;
}

void Renderer::bind_graphics_pipeline(GraphicsPipelineHandle handle)
{
    m_current_draw_state.command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipelines.at(handle));
}

void Renderer::push_to_all_frames(std::function<void(uint32_t)> func)
{
    uint32_t id = m_deferred_function_id_count;
    m_deferred_function_id_count++;
    m_deferred_functions.insert({ id, { std::move(func), c_frames_in_flight } });
    for (FrameInFlight& frame : m_frames_in_flight) {
        frame.funcs.push(id);
    }
}

void Renderer::push_to_next_frame(std::function<void(uint32_t)> func)
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

void Renderer::push_wait_for_frames(std::function<void(uint32_t)> func)
{
    uint32_t id = m_deferred_function_id_count;
    m_deferred_function_id_count++;
    m_deferred_functions.insert({ id, { std::move(func), c_frames_in_flight } });
    m_wait_frames_deferred_functions.push(id);
}

vk::CommandBuffer Renderer::begin_single_submit(vk::Device device, vk::CommandPool pool)
{
    auto command_buffer_alloc_info
        = vk::CommandBufferAllocateInfo()
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandPool(pool)
              .setCommandBufferCount(1);

    vk::ResultValue<std::vector<vk::CommandBuffer>> command_buffer_result
        = device.allocateCommandBuffers(command_buffer_alloc_info);
    if (command_buffer_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to allocate texture command buffer.");
    }
    vk::CommandBuffer command_buffer = std::move(command_buffer_result.value.at(0));

    auto begin_info = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    vk::Result begin_result = command_buffer.begin(begin_info);
    if (begin_result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to begin single submit buffer.");
    }

    return command_buffer;
}

void Renderer::end_single_submit(
    vk::Device device, vk::CommandPool pool, vk::CommandBuffer command_buffer, vk::Queue queue)
{
    vk::Result end_result = command_buffer.end();
    if (end_result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to end single submit buffer.");
    }

    auto submit_info = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&command_buffer);

    vk::Result submit_result = queue.submit(1, &submit_info, VK_NULL_HANDLE);
    if (submit_result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to submit single submit buffer.");
    }
    vk::Result wait_result = queue.waitIdle();
    if (wait_result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to wait for queue for single submit.");
    }

    device.freeCommandBuffers(pool, 1, &command_buffer);
}

void Renderer::transition_image_layout(
    vk::Device device,
    vk::CommandPool pool,
    vk::Queue queue,
    vk::Image image,
    vk::Format format,
    vk::ImageLayout old_layout,
    vk::ImageLayout new_layout)
{
    vk::CommandBuffer command_buffer = begin_single_submit(device, pool);

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
                                       .setLevelCount(1)
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
        throw std::runtime_error("[Renderer] Unsupported layout transition.");
    }

    command_buffer.pipelineBarrier(
        source_stage, destination_stage, vk::DependencyFlagBits::eByRegion, 0, nullptr, 0, nullptr, 1, &barrier);

    end_single_submit(device, pool, command_buffer, queue);
}

void Renderer::copy_buffer_to_image(
    vk::Device device,
    vk::CommandPool pool,
    vk::Queue queue,
    vk::Buffer buffer,
    vk::Image image,
    uint32_t width,
    uint32_t height)
{
    vk::CommandBuffer command_buffer = begin_single_submit(device, pool);

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

    command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

    end_single_submit(device, pool, command_buffer, queue);
}

vk::ImageView Renderer::create_image_view(
    vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags)
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
              .setLevelCount(1)
              .setBaseArrayLayer(0)
              .setLayerCount(1);

    auto view_info
        = vk::ImageViewCreateInfo()
              .setImage(image)
              .setViewType(vk::ImageViewType::e2D)
              .setFormat(format)
              .setComponents(components)
              .setSubresourceRange(image_subresource_range);

    vk::ResultValue<vk::ImageView> image_view_result = device.createImageView(view_info);
    if (image_view_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create image view.");
    }
    return image_view_result.value;
}

vk::Sampler Renderer::create_texture_sampler(vk::PhysicalDevice physical_device, vk::Device device)
{
    auto sampler_info
        = vk::SamplerCreateInfo()
              .setMagFilter(vk::Filter::eLinear)
              .setMinFilter(vk::Filter::eLinear)
              .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
              .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
              .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
              .setAnisotropyEnable(VK_TRUE) // Disable with VK_FALSE
              .setMaxAnisotropy(physical_device.getProperties().limits.maxSamplerAnisotropy) // Disable with 1.0f
              .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
              .setUnnormalizedCoordinates(VK_FALSE)
              .setCompareEnable(VK_FALSE)
              .setCompareOp(vk::CompareOp::eAlways)
              .setMipmapMode(vk::SamplerMipmapMode::eLinear)
              .setMipLodBias(0.0f)
              .setMinLod(0.0f)
              .setMaxLod(0.0f);

    vk::ResultValue<vk::Sampler> sampler_result = device.createSampler(sampler_info);
    if (sampler_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create image sampler.");
    }
    return sampler_result.value;
}

TextureHandle Renderer::create_texture(
    const std::filesystem::path& path, DescriptorSetHandle descriptor_set, uint32_t binding)
{
    int width;
    int height;
    int channels;
    std::string path_string = path.string();
    stbi_uc* pixels = stbi_load(path_string.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    vk::DeviceSize size = width * height * 4;
    if (!pixels) {
        throw std::runtime_error("[Renderer] Failed to load texture image.");
    }

    Buffer staging_buffer = create_buffer(
        m_vma_allocator,
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* data;
    vmaMapMemory(m_vma_allocator, staging_buffer.vma_allocation, &data);
    memcpy(data, pixels, static_cast<size_t>(size));
    vmaUnmapMemory(m_vma_allocator, staging_buffer.vma_allocation);

    stbi_image_free(pixels);

    Image image = create_image(
        m_vma_allocator,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

    transition_image_layout(
        m_vk_device,
        m_vk_command_pool,
        m_vk_graphics_queue,
        image.vk_handle,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal);

    copy_buffer_to_image(
        m_vk_device,
        m_vk_command_pool,
        m_vk_graphics_queue,
        staging_buffer.vk_handle,
        image.vk_handle,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height));

    transition_image_layout(
        m_vk_device,
        m_vk_command_pool,
        m_vk_graphics_queue,
        image.vk_handle,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal);

    vmaDestroyBuffer(m_vma_allocator, staging_buffer.vk_handle, staging_buffer.vma_allocation);

    vk::ImageView image_view
        = create_image_view(m_vk_device, image.vk_handle, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);

    vk::Sampler sampler = create_texture_sampler(m_vk_physical_device, m_vk_device);

    Texture texture { .image = image, .vk_image_view = image_view, .vk_sampler = sampler };

    push_to_all_frames([this, texture, descriptor_set](uint32_t frame_index) {
        auto image_info = vk::DescriptorImageInfo()
                              .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                              .setImageView(texture.vk_image_view)
                              .setSampler(texture.vk_sampler);

        auto descriptor_write
            = vk::WriteDescriptorSet()
                  .setDstSet(this->m_frames_in_flight.at(frame_index).descriptor_sets.at(descriptor_set))
                  .setDstBinding(1)
                  .setDstArrayElement(0)
                  .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                  .setDescriptorCount(1)
                  .setPImageInfo(&image_info);

        this->m_vk_device.updateDescriptorSets(1, &descriptor_write, 0, nullptr);
    });

    TextureHandle handle = TextureHandle(m_resource_handle_count);
    m_resource_handle_count++;
    m_textures.insert({ handle, texture });
    return handle;
}

Renderer::DepthImage Renderer::create_depth_image(
    vk::PhysicalDevice physical_device,
    vk::Device device,
    vk::CommandPool pool,
    vk::Queue queue,
    VmaAllocator allocator,
    vk::Extent2D extent)
{
    vk::Format depth_format = find_depth_format(physical_device);

    Image depth_image = create_image(
        allocator,
        extent.width,
        extent.height,
        depth_format,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment);

    vk::ImageView depth_image_view
        = create_image_view(device, depth_image.vk_handle, depth_format, vk::ImageAspectFlagBits::eDepth);

    transition_image_layout(
        device,
        pool,
        queue,
        depth_image.vk_handle,
        depth_format,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal);

    return { depth_image, depth_image_view };
}

vk::Format Renderer::find_supported_format(
    vk::PhysicalDevice physical_device,
    const std::vector<vk::Format>& formats,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlags features)
{
    for (vk::Format format : formats) {
        vk::FormatProperties properties = physical_device.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("[Renderer] Failed to find supported format.");
}

vk::Format Renderer::find_depth_format(vk::PhysicalDevice physical_device)
{
    const std::vector<vk::Format> formats
        = { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };
    return find_supported_format(
        physical_device, formats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool Renderer::has_stencil_component(vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

Renderer::Image Renderer::create_image(
    VmaAllocator allocator,
    uint32_t width,
    uint32_t height,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage)
{
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = static_cast<VkFormat>(format);
    image_info.tiling = static_cast<VkImageTiling>(tiling);
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = static_cast<VkImageUsageFlags>(usage);
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo vma_alloc_info = {};
    vma_alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    vma_alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    VkImage image;
    VmaAllocation image_allocation;
    vmaCreateImage(allocator, &image_info, &vma_alloc_info, &image, &image_allocation, nullptr);
    return { image, image_allocation };
};

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
{
}

vk::DescriptorPool Renderer::DescriptorSetAllocator::create_pool(vk::Device device, vk::DescriptorPoolCreateFlags flags)
{
    std::vector<vk::DescriptorPoolSize> sizes;
    sizes.reserve(m_sizes.size());

    for (std::pair<vk::DescriptorType, float> s : m_sizes) {
        sizes.push_back({ s.first, uint32_t(s.second * m_max_sets_per_pool) });
    }

    auto pool_info = vk::DescriptorPoolCreateInfo()
                         .setFlags(flags)
                         .setMaxSets(static_cast<uint32_t>(m_max_sets_per_pool))
                         .setPoolSizes(sizes);

    vk::ResultValue<vk::DescriptorPool> descriptor_pool_result = device.createDescriptorPool(pool_info);
    if (descriptor_pool_result.result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Failed to create descriptor pool.");
    }
    return descriptor_pool_result.value;
}

void Renderer::DescriptorSetAllocator::cleanup(vk::Device device)
{
    for (auto& [descriptor_set, descriptor_pool] : m_descriptor_sets) {
        device.freeDescriptorSets(descriptor_pool, 1, &descriptor_set);
    }
    for (vk::DescriptorPool descriptor_pool : m_descriptor_pools) {
        device.destroy(descriptor_pool);
    }
}

vk::DescriptorSet Renderer::DescriptorSetAllocator::create(vk::Device device, vk::DescriptorSetLayout layout)
{
    if (m_descriptor_pools.empty()) {
        m_descriptor_pools.push_back(create_pool(device, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet));
        m_current_pool_index = 0;
    }

    std::optional<vk::DescriptorSet> descriptor_set
        = try_create(m_descriptor_pools.at(m_current_pool_index), device, layout);

    if (!descriptor_set.has_value()) {
        for (size_t i = 0; i < m_descriptor_pools.size(); i++) {
            if (i == m_current_pool_index) {
                continue;
            }
            descriptor_set = try_create(m_descriptor_pools.at(i), device, layout);
            if (descriptor_set.has_value()) {
                m_current_pool_index = i;
                break;
            }
        }

        if (!descriptor_set.has_value()) {
            m_descriptor_pools.push_back(create_pool(device, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet));
            m_current_pool_index = m_descriptor_pools.size() - 1;
            descriptor_set = try_create(m_descriptor_pools.at(m_current_pool_index), device, layout);

            if (!descriptor_set.has_value()) {
                throw std::runtime_error("[Renderer] Failed to allocate descriptor set");
            }
        }
    }

    m_descriptor_sets.push_back({ descriptor_set.value(), m_descriptor_pools.at(m_current_pool_index) });

    return descriptor_set.value();
}

std::optional<vk::DescriptorSet> Renderer::DescriptorSetAllocator::try_create(
    vk::DescriptorPool pool, vk::Device device, vk::DescriptorSetLayout layout)
{
    auto alloc_info
        = vk::DescriptorSetAllocateInfo().setDescriptorPool(pool).setDescriptorSetCount(1).setPSetLayouts(&layout);

    vk::ResultValue<std::vector<vk::DescriptorSet>> descriptor_sets_result = device.allocateDescriptorSets(alloc_info);

    if (descriptor_sets_result.result == vk::Result::eErrorOutOfPoolMemory
        || descriptor_sets_result.result == vk::Result::eErrorFragmentedPool) {
        return {};
    }
    else if (descriptor_sets_result.result == vk::Result::eSuccess) {
        return descriptor_sets_result.value.at(0);
    }
    else {
        throw std::runtime_error("[Renderer] Failed to allocate descriptor sets.");
    }
}
}