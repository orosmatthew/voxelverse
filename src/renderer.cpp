#include "renderer.hpp"

#include <fstream>
#include <set>
#include <vector>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <glm/gtc/matrix_transform.hpp>

#include "logger.hpp"
#include "shader.hpp"
#include "window.hpp"

namespace mve {
Renderer::Renderer(
    const Window& window,
    const std::string& app_name,
    int app_version_major,
    int app_version_minor,
    int app_version_patch,
    const Shader& vertex_shader,
    const Shader& fragment_shader,
    const VertexLayout& layout,
    int frames_in_flight)
    : c_frames_in_flight(frames_in_flight)
    , m_resource_handle_count(0)
{
    m_vk_instance = create_vk_instance(app_name, app_version_major, app_version_minor, app_version_patch);
#ifdef MVE_ENABLE_VALIDATION_LAYERS
    m_vk_debug_utils_messenger = create_vk_debug_messenger(m_vk_instance);
#endif
    m_vk_surface = create_vk_surface(m_vk_instance, window.get_glfw_handle());
    m_vk_physical_device = pick_vk_physical_device(m_vk_instance, m_vk_surface);
    m_vk_queue_family_indices = get_vk_queue_family_indices(m_vk_physical_device, m_vk_surface);
    m_vk_device = create_vk_logical_device(m_vk_physical_device, m_vk_queue_family_indices);

    SwapchainSupportDetails swapchain_support_details
        = get_vk_swapchain_support_details(m_vk_physical_device, m_vk_surface);

    m_vk_swapchain_image_format = choose_vk_swapchain_surface_format(swapchain_support_details.formats);
    m_vk_swapchain_extent = get_vk_swapchain_extent(swapchain_support_details.capabilities, window.get_glfw_handle());
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

    m_vk_descriptor_set_layout = create_vk_descriptor_set_layout(m_vk_device);

    m_vk_pipeline_layout = create_vk_pipeline_layout(m_vk_device, m_vk_descriptor_set_layout);

    m_vk_render_pass = create_vk_render_pass(m_vk_device, m_vk_swapchain_image_format.format);

    m_vk_graphics_pipeline = create_vk_graphics_pipeline(
        m_vk_device, vertex_shader, fragment_shader, m_vk_pipeline_layout, m_vk_render_pass, layout);

    m_vk_swapchain_framebuffers
        = create_vk_framebuffers(m_vk_device, m_vk_swapchain_image_views, m_vk_render_pass, m_vk_swapchain_extent);

    m_vk_command_pool = create_vk_command_pool(m_vk_device, m_vk_queue_family_indices);

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = m_vk_physical_device;
    allocatorCreateInfo.device = m_vk_device;
    allocatorCreateInfo.instance = m_vk_instance;

    vmaCreateAllocator(&allocatorCreateInfo, &m_vma_allocator);

    m_frames_in_flight = create_frames_in_flight(m_vk_device, m_vk_command_pool, c_frames_in_flight);

    m_vk_descriptor_pool = create_vk_descriptor_pool(m_vk_device, c_frames_in_flight);

    m_current_draw_state.is_drawing = false;
    m_current_draw_state.frame_index = 0;
    m_current_draw_state.image_index = 0;
    m_current_draw_state.command_buffer = VK_NULL_HANDLE;
}

bool Renderer::has_validation_layer_support()
{
    std::vector<vk::LayerProperties> available_layers = vk::enumerateInstanceLayerProperties();

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
              .setApiVersion(VK_API_VERSION_1_0);

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

    return vk::createInstance(instance_create_info);
}

std::vector<const char*> Renderer::get_vk_validation_layer_exts()
{
    return std::vector<const char*> { "VK_LAYER_KHRONOS_validation" };
}

vk::PhysicalDevice Renderer::pick_vk_physical_device(vk::Instance instance, vk::SurfaceKHR surface)
{
    std::vector<vk::PhysicalDevice> physical_devices = instance.enumeratePhysicalDevices();

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

    std::vector<vk::ExtensionProperties> available_exts = physical_device.enumerateDeviceExtensionProperties();

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

    return indices.is_complete() && is_swapchain_adequate;
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

            if (physical_device.getSurfaceSupportKHR(i, surface)) {
                indices.present_family = i;
            }

            if (indices.is_complete()) {
                break;
            }
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

    return physical_device.createDevice(device_create_info);
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

    details.capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
    details.formats = physical_device.getSurfaceFormatsKHR(surface);
    details.present_modes = physical_device.getSurfacePresentModesKHR(surface);

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
    return device.createSwapchainKHR(swapchain_create_info);
}

std::vector<vk::Image> Renderer::get_vk_swapchain_images(vk::Device device, vk::SwapchainKHR swapchain)
{
    return device.getSwapchainImagesKHR(swapchain);
}

std::vector<vk::ImageView> Renderer::create_vk_swapchain_image_views(
    vk::Device device, const std::vector<vk::Image>& swapchain_images, vk::Format image_format)
{
    std::vector<vk::ImageView> image_views;

    for (vk::Image swapchain_image : swapchain_images) {
        auto image_view_create_info
            = vk::ImageViewCreateInfo()
                  .setImage(swapchain_image)
                  .setViewType(vk::ImageViewType::e2D)
                  .setFormat(image_format)
                  .setComponents(vk::ComponentMapping()
                                     .setR(vk::ComponentSwizzle::eIdentity)
                                     .setG(vk::ComponentSwizzle::eIdentity)
                                     .setB(vk::ComponentSwizzle::eIdentity)
                                     .setA(vk::ComponentSwizzle::eIdentity))
                  .setSubresourceRange(vk::ImageSubresourceRange()
                                           .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                           .setBaseMipLevel(0)
                                           .setLevelCount(1)
                                           .setBaseArrayLayer(0)
                                           .setLayerCount(1));

        image_views.push_back(device.createImageView(image_view_create_info));
    }

    return image_views;
}

vk::Pipeline Renderer::create_vk_graphics_pipeline(
    vk::Device device,
    const Shader& vertex_shader,
    const Shader& fragment_shader,
    vk::PipelineLayout pipeline_layout,
    vk::RenderPass render_pass,
    const VertexLayout& layout)
{
    std::vector<std::byte> vertex_spv_code = vertex_shader.spv_code();
    auto vertex_shader_create_info
        = vk::ShaderModuleCreateInfo()
              .setCodeSize(vertex_spv_code.size())
              .setPCode(reinterpret_cast<const uint32_t*>(vertex_spv_code.data()));

    vk::ShaderModule vertex_shader_module = device.createShaderModule(vertex_shader_create_info);

    std::vector<std::byte> fragment_spv_code = fragment_shader.spv_code();
    auto fragment_shader_create_info
        = vk::ShaderModuleCreateInfo()
              .setCodeSize(fragment_spv_code.size())
              .setPCode(reinterpret_cast<const uint32_t*>(fragment_spv_code.data()));

    vk::ShaderModule fragment_shader_module = device.createShaderModule(fragment_shader_create_info);

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

    vk::VertexInputBindingDescription binding_description = create_vk_binding_description(layout);
    std::vector<vk::VertexInputAttributeDescription> attribute_descriptions = create_vk_attribute_descriptions(layout);

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
              .setPDepthStencilState(nullptr)
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

vk::PipelineLayout Renderer::create_vk_pipeline_layout(vk::Device device, vk::DescriptorSetLayout descriptor_set_layout)
{
    auto pipeline_layout_info
        = vk::PipelineLayoutCreateInfo()
              .setSetLayoutCount(1)
              .setPSetLayouts(&descriptor_set_layout)
              .setPushConstantRangeCount(0)
              .setPPushConstantRanges(nullptr);

    return device.createPipelineLayout(pipeline_layout_info);
}

vk::RenderPass Renderer::create_vk_render_pass(vk::Device device, vk::Format swapchain_format)
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

    auto color_attachment_ref
        = vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    auto subpass = vk::SubpassDescription()
                       .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                       .setColorAttachmentCount(1)
                       .setPColorAttachments(&color_attachment_ref);

    auto subpass_dependency
        = vk::SubpassDependency()
              .setSrcSubpass(VK_SUBPASS_EXTERNAL)
              .setDstSubpass(0)
              .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
              .setSrcAccessMask(vk::AccessFlagBits::eNone)
              .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
              .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    auto render_pass_info
        = vk::RenderPassCreateInfo()
              .setAttachmentCount(1)
              .setPAttachments(&color_attachment)
              .setSubpassCount(1)
              .setPSubpasses(&subpass)
              .setSubpassCount(1)
              .setPDependencies(&subpass_dependency);

    return device.createRenderPass(render_pass_info);
}

std::vector<vk::Framebuffer> Renderer::create_vk_framebuffers(
    vk::Device device,
    const std::vector<vk::ImageView>& swapchain_image_views,
    vk::RenderPass render_pass,
    vk::Extent2D swapchain_extent)
{
    std::vector<vk::Framebuffer> framebuffers;

    for (const vk::ImageView& swapchain_image_view : swapchain_image_views) {
        vk::ImageView attachments[] = { swapchain_image_view };

        auto framebuffer_info
            = vk::FramebufferCreateInfo()
                  .setRenderPass(render_pass)
                  .setAttachmentCount(1)
                  .setPAttachments(attachments)
                  .setWidth(swapchain_extent.width)
                  .setHeight(swapchain_extent.height)
                  .setLayers(1);

        framebuffers.push_back(device.createFramebuffer(framebuffer_info));
    }
    return framebuffers;
}

vk::CommandPool Renderer::create_vk_command_pool(vk::Device device, QueueFamilyIndices queue_family_indices)
{
    assert(queue_family_indices.is_complete());

    auto command_pool_info = vk::CommandPoolCreateInfo()
                                 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                                 .setQueueFamilyIndex(queue_family_indices.graphics_family.value());

    return device.createCommandPool(command_pool_info);
}

std::vector<vk::CommandBuffer> Renderer::create_vk_command_buffers(
    vk::Device device, vk::CommandPool command_pool, int frames_in_flight)
{
    auto buffer_alloc_info
        = vk::CommandBufferAllocateInfo()
              .setCommandPool(command_pool)
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandBufferCount(static_cast<uint32_t>(frames_in_flight));

    std::vector<vk::CommandBuffer> command_buffers = device.allocateCommandBuffers(buffer_alloc_info);
    return command_buffers;
}

Renderer::~Renderer()
{
#ifdef MVE_ENABLE_VALIDATION_LAYERS
    cleanup_vk_debug_messenger();
#endif
    m_vk_device.waitIdle();

    cleanup_vk_swapchain();

    m_vk_device.destroy(m_vk_descriptor_pool);

    for (FrameInFlight& frame : m_frames_in_flight) {
        for (auto& pair : frame.uniform_buffers) {
            vmaUnmapMemory(m_vma_allocator, pair.second.buffer.vma_allocation);
            vmaDestroyBuffer(m_vma_allocator, pair.second.buffer.vk_handle, pair.second.buffer.vma_allocation);
        }
    }

    m_vk_device.destroy(m_vk_descriptor_set_layout);

    for (auto& buffer : m_vertex_buffers) {
        vmaDestroyBuffer(m_vma_allocator, buffer.second.buffer.vk_handle, buffer.second.buffer.vma_allocation);
    }

    for (auto& buffer : m_index_buffers) {
        vmaDestroyBuffer(m_vma_allocator, buffer.second.buffer.vk_handle, buffer.second.buffer.vma_allocation);
    }

    vmaDestroyAllocator(m_vma_allocator);

    m_vk_device.destroy(m_vk_graphics_pipeline);
    m_vk_device.destroy(m_vk_pipeline_layout);
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
    glm::ivec2 window_size = window.get_size();

    while (window_size == glm::ivec2(0, 0)) {
        window_size = window.get_size();
        window.wait_for_events();
    }

    m_vk_device.waitIdle();

    cleanup_vk_swapchain();

    SwapchainSupportDetails swapchain_support_details
        = get_vk_swapchain_support_details(m_vk_physical_device, m_vk_surface);

    m_vk_swapchain_extent = get_vk_swapchain_extent(swapchain_support_details.capabilities, window.get_glfw_handle());

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

    m_vk_swapchain_framebuffers
        = create_vk_framebuffers(m_vk_device, m_vk_swapchain_image_views, m_vk_render_pass, m_vk_swapchain_extent);
}

void Renderer::cleanup_vk_swapchain()
{
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

    exts.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

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

Renderer::VertexBufferHandle Renderer::upload(const VertexData& vertex_data)
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
        case VertexAttributeType::e_float:
            description.setFormat(vk::Format::eR32Sfloat);
            offset += sizeof(float);
            break;
        case VertexAttributeType::e_vec2:
            description.setFormat(vk::Format::eR32G32Sfloat);
            offset += sizeof(glm::vec2);
            break;
        case VertexAttributeType::e_vec3:
            description.setFormat(vk::Format::eR32G32B32Sfloat);
            offset += sizeof(glm::vec3);
            break;
        case VertexAttributeType::e_vec4:
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
        frame.image_available_semaphore = device.createSemaphore(semaphore_info);
        frame.render_finished_semaphore = device.createSemaphore(semaphore_info);
        frame.in_flight_fence = device.createFence(fence_info);
        frame.command_buffer = command_buffers.at(i);
        frame.uniform_buffers = {};
        frames_in_flight.push_back(frame);
    }

    return frames_in_flight;
}

void Renderer::queue_destroy(Renderer::VertexBufferHandle handle)
{
    m_vertex_buffer_deletion_queue[handle] = 0;
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
    auto alloc_info = vk::CommandBufferAllocateInfo()
                          .setLevel(vk::CommandBufferLevel::ePrimary)
                          .setCommandPool(command_pool)
                          .setCommandBufferCount(1);

    vk::CommandBuffer command_buffer = device.allocateCommandBuffers(alloc_info)[0];

    auto begin_info = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    command_buffer.begin(begin_info);

    auto copy_region = vk::BufferCopy().setSrcOffset(0).setDstOffset(0).setSize(size);
    command_buffer.copyBuffer(src_buffer, dst_buffer, 1, &copy_region);

    command_buffer.end();

    auto submit_info = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&command_buffer);
    vk::Result result = graphics_queue.submit(1, &submit_info, VK_NULL_HANDLE);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("[Renderer] Buffer copy submission failed.");
    }
    graphics_queue.waitIdle();

    device.freeCommandBuffers(command_pool, 1, &command_buffer);
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

Renderer::IndexBufferHandle Renderer::upload(const std::vector<uint32_t>& index_data)
{
    IndexBuffer index_buffer
        = create_index_buffer(m_vk_device, m_vk_command_pool, m_vk_graphics_queue, m_vma_allocator, index_data);
    m_index_buffers[IndexBufferHandle(m_resource_handle_count)] = index_buffer;
    m_resource_handle_count++;

    return IndexBufferHandle(m_resource_handle_count - 1);
}

void Renderer::queue_destroy(Renderer::IndexBufferHandle handle)
{
    m_index_buffer_deletion_queue[handle] = 0;
}

void Renderer::begin(const Window& window)
{
    if (m_current_draw_state.is_drawing) {
        throw std::runtime_error("[Renderer] Already drawing.");
    }

    m_current_draw_state.is_drawing = true;

    if (window.was_resized()) {
        recreate_swapchain(window);
    }

    FrameInFlight& frame = m_frames_in_flight[m_current_draw_state.frame_index];

    wait_ready();

    auto destroyed = std::vector<VertexBufferHandle>();
    for (auto& handle_pair : m_vertex_buffer_deletion_queue) {
        if (handle_pair.second < c_frames_in_flight) {
            handle_pair.second++;
            break;
        }
        else {
            vmaDestroyBuffer(
                m_vma_allocator,
                m_vertex_buffers.at(handle_pair.first).buffer.vk_handle,
                m_vertex_buffers.at(handle_pair.first).buffer.vma_allocation);
            m_vertex_buffers.erase(handle_pair.first);
            destroyed.push_back(handle_pair.first);
        }
    }
    for (VertexBufferHandle v : destroyed) {
        m_vertex_buffer_deletion_queue.erase(v);
    }

    auto index_destroyed = std::vector<IndexBufferHandle>();
    for (auto& handle_pair : m_index_buffer_deletion_queue) {
        if (handle_pair.second < c_frames_in_flight) {
            handle_pair.second++;
            break;
        }
        else {
            vmaDestroyBuffer(
                m_vma_allocator,
                m_index_buffers.at(handle_pair.first).buffer.vk_handle,
                m_index_buffers.at(handle_pair.first).buffer.vma_allocation);
            m_index_buffers.erase(handle_pair.first);
            index_destroyed.push_back(handle_pair.first);
        }
    }
    for (IndexBufferHandle v : index_destroyed) {
        m_index_buffer_deletion_queue.erase(v);
    }

    vk::ResultValue<uint32_t> acquire_result
        = m_vk_device.acquireNextImageKHR(m_vk_swapchain, UINT64_MAX, frame.image_available_semaphore, nullptr);
    if (acquire_result.result != vk::Result::eSuccess && acquire_result.result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }
    m_current_draw_state.image_index = acquire_result.value;

    m_vk_device.resetFences({ frame.in_flight_fence });

    frame.command_buffer.reset();

    m_current_draw_state.command_buffer = m_frames_in_flight[m_current_draw_state.frame_index].command_buffer;

    auto buffer_begin_info = vk::CommandBufferBeginInfo();
    m_current_draw_state.command_buffer.begin(buffer_begin_info);

    auto clear_color = vk::ClearValue(vk::ClearColorValue(std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f }));

    auto render_pass_begin_info
        = vk::RenderPassBeginInfo()
              .setRenderPass(m_vk_render_pass)
              .setFramebuffer(m_vk_swapchain_framebuffers[m_current_draw_state.image_index])
              .setRenderArea(vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_vk_swapchain_extent))
              .setClearValueCount(1)
              .setPClearValues(&clear_color);

    m_current_draw_state.command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

    m_current_draw_state.command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_vk_graphics_pipeline);

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

    m_current_draw_state.command_buffer.end();

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

    m_vk_graphics_queue.submit({ submit_info }, frame.in_flight_fence);

    vk::SwapchainKHR swapchains[] = { m_vk_swapchain };

    auto present_info
        = vk::PresentInfoKHR()
              .setWaitSemaphores(signal_semaphores)
              .setSwapchains(swapchains)
              .setPImageIndices(&(m_current_draw_state.image_index));

    vk::Result present_result = m_vk_present_queue.presentKHR(present_info);

    if (present_result == vk::Result::eSuboptimalKHR) {
        recreate_swapchain(window);
    }
    else if (present_result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present frame");
    }

    m_current_draw_state.frame_index = (m_current_draw_state.frame_index + 1) % c_frames_in_flight;

    m_current_draw_state.is_drawing = false;
}

void Renderer::draw(VertexBufferHandle handle)
{
    VertexBuffer& vertex_buffer = m_vertex_buffers.at(handle);
    m_current_draw_state.command_buffer.bindVertexBuffers(0, vertex_buffer.buffer.vk_handle, { 0 });
    m_current_draw_state.command_buffer.draw(vertex_buffer.vertex_count, 1, 0, 0);
}

void Renderer::bind(Renderer::VertexBufferHandle handle)
{
    m_current_draw_state.command_buffer.bindVertexBuffers(0, m_vertex_buffers.at(handle).buffer.vk_handle, { 0 });
}

void Renderer::draw(Renderer::IndexBufferHandle handle)
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

    return device.createDescriptorSetLayout(layout_info);
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

    return device.createDescriptorPool(pool_info);
}

std::vector<vk::DescriptorSet> Renderer::create_vk_descriptor_sets(
    vk::Device device, vk::DescriptorSetLayout layout, vk::DescriptorPool pool, int count)
{
    auto layouts = std::vector<vk::DescriptorSetLayout>(count, layout);

    auto alloc_info = vk::DescriptorSetAllocateInfo()
                          .setDescriptorPool(pool)
                          .setDescriptorSetCount(static_cast<uint32_t>(count))
                          .setPSetLayouts(layouts.data());

    std::vector<vk::DescriptorSet> descriptor_sets = device.allocateDescriptorSets(alloc_info);

    return descriptor_sets;
}

bool Renderer::is_valid(Renderer::VertexBufferHandle handle)
{
    return m_vertex_buffers.contains(handle) && !m_vertex_buffer_deletion_queue.contains(handle);
}

bool Renderer::is_valid(Renderer::IndexBufferHandle handle)
{
    return m_index_buffers.contains(handle) && !m_index_buffer_deletion_queue.contains(handle);
}

Renderer::UniformBufferHandle Renderer::create_uniform_buffer(const UniformStructLayout& struct_layout)
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

    std::vector<vk::DescriptorSet> descriptor_sets
        = create_vk_descriptor_sets(m_vk_device, m_vk_descriptor_set_layout, m_vk_descriptor_pool, c_frames_in_flight);
    for (int i = 0; i < c_frames_in_flight; i++) {
        m_frames_in_flight[i].descriptor_sets[handle] = descriptor_sets.at(i);

        auto buffer_info = vk::DescriptorBufferInfo()
                               .setBuffer(m_frames_in_flight[i].uniform_buffers[handle].buffer.vk_handle)
                               .setOffset(0)
                               .setRange(struct_layout.size_bytes());

        auto descriptor_write
            = vk::WriteDescriptorSet()
                  .setDstSet(m_frames_in_flight[i].descriptor_sets[handle])
                  .setDstBinding(0)
                  .setDstArrayElement(0)
                  .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                  .setDescriptorCount(1)
                  .setPBufferInfo(&buffer_info);

        m_vk_device.updateDescriptorSets(1, &descriptor_write, 0, nullptr);
    }
    return handle;
}

void Renderer::update_uniform(Renderer::UniformBufferHandle handle, UniformLocation location, glm::mat4 value)
{
    UniformBuffer& buffer = m_frames_in_flight[m_current_draw_state.frame_index].uniform_buffers[handle];

    memcpy(&(buffer.mapped_ptr[location.value_of()]), &value, sizeof(glm::mat4));
}

void Renderer::bind(Renderer::UniformBufferHandle handle)
{
    m_current_draw_state.command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        m_vk_pipeline_layout,
        0,
        1,
        &(m_frames_in_flight[m_current_draw_state.frame_index].descriptor_sets[handle]),
        0,
        nullptr);
}

glm::ivec2 Renderer::get_extent() const
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
}
