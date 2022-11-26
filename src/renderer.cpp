#include "renderer.hpp"
#include <fstream>
#include <set>
#include <shaderc/shaderc.hpp>

#include "window.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <unordered_map>

namespace mve {
    Renderer::Renderer(
        const Window &window,
        const std::string &app_name,
        int app_version_major,
        int app_version_minor,
        int app_version_patch,
        const Shader &vertex_shader,
        const Shader &fragment_shader,
        const VertexLayout &layout,
        int frames_in_flight)
        : c_frames_in_flight(frames_in_flight)
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
        m_vk_swapchain_extent
            = get_vk_swapchain_extent(swapchain_support_details.capabilities, window.get_glfw_handle());
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

        m_vk_pipeline_layout = create_vk_pipeline_layout(m_vk_device);

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
    }

    bool Renderer::has_validation_layer_support()
    {
        std::vector<vk::LayerProperties> available_layers = vk::enumerateInstanceLayerProperties();

        const std::vector<const char *> validation_layers = get_vk_validation_layer_exts();

        for (const std::string &validation_layer : validation_layers) {
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
        const std::string &app_name, int app_version_major, int app_version_minor, int app_version_patch)
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

        std::vector<const char *> exts = get_vk_instance_required_exts();

#ifdef MVE_ENABLE_VALIDATION_LAYERS

        const std::vector<const char *> validation_layers = get_vk_validation_layer_exts();

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

    std::vector<const char *> Renderer::get_vk_validation_layer_exts()
    {
        return std::vector<const char *> { "VK_LAYER_KHRONOS_validation" };
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

        std::vector<const char *> required_exts = get_vk_device_required_exts();

        for (const std::string &required_ext : required_exts) {
            bool is_available = false;
            for (const vk::ExtensionProperties &ext_props : available_exts) {
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
        for (const vk::QueueFamilyProperties &queue_family : queue_families) {
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

        std::vector<const char *> required_exts = get_vk_device_required_exts();

#ifdef MVE_ENABLE_VALIDATION_LAYERS
        const std::vector<const char *> validation_layers = get_vk_validation_layer_exts();

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
    vk::SurfaceKHR Renderer::create_vk_surface(vk::Instance instance, GLFWwindow *window)
    {
        VkSurfaceKHR surface;
        VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }
        return { surface };
    }

    std::vector<const char *> Renderer::get_vk_device_required_exts()
    {
        return std::vector<const char *> { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
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
        const std::vector<vk::SurfaceFormatKHR> &available_formats)
    {
        for (const vk::SurfaceFormatKHR &available_format : available_formats) {
            if (available_format.format == vk::Format::eB8G8R8A8Srgb
                && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return available_format;
            }
        }
        return available_formats[0];
    }
    vk::PresentModeKHR Renderer::choose_vk_swapchain_present_mode(
        const std::vector<vk::PresentModeKHR> &available_present_modes)
    {
        for (const vk::PresentModeKHR &available_present_mode : available_present_modes) {
            if (available_present_mode == vk::PresentModeKHR::eMailbox) {
                return available_present_mode;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }
    vk::Extent2D Renderer::get_vk_swapchain_extent(const vk::SurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
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
            actual_extent.height = std::clamp(
                actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

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
        vk::Device device, const std::vector<vk::Image> &swapchain_images, vk::Format image_format)
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
        const Shader &vertex_shader,
        const Shader &fragment_shader,
        vk::PipelineLayout pipeline_layout,
        vk::RenderPass render_pass,
        const VertexLayout &layout)
    {
        std::vector<char> vertex_spv_code = vertex_shader.get_spv_code();
        auto vertex_shader_create_info
            = vk::ShaderModuleCreateInfo()
                  .setCodeSize(vertex_spv_code.size())
                  .setPCode(reinterpret_cast<const uint32_t *>(vertex_spv_code.data()));

        vk::ShaderModule vertex_shader_module = device.createShaderModule(vertex_shader_create_info);

        std::vector<char> fragment_spv_code = fragment_shader.get_spv_code();
        auto fragment_shader_create_info
            = vk::ShaderModuleCreateInfo()
                  .setCodeSize(fragment_spv_code.size())
                  .setPCode(reinterpret_cast<const uint32_t *>(fragment_spv_code.data()));

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
        std::vector<vk::VertexInputAttributeDescription> attribute_descriptions
            = create_vk_attribute_descriptions(layout);

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
                  .setFrontFace(vk::FrontFace::eClockwise)
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

    vk::PipelineLayout Renderer::create_vk_pipeline_layout(vk::Device device)
    {
        auto pipeline_layout_info
            = vk::PipelineLayoutCreateInfo()
                  .setSetLayoutCount(0)
                  .setPSetLayouts(nullptr)
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
        const std::vector<vk::ImageView> &swapchain_image_views,
        vk::RenderPass render_pass,
        vk::Extent2D swapchain_extent)
    {
        std::vector<vk::Framebuffer> framebuffers;

        for (const vk::ImageView &swapchain_image_view : swapchain_image_views) {
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

    void Renderer::draw_frame(const Window &window)
    {
        if (window.was_resized()) {
            recreate_swapchain(window);
        }

        FrameInFlight &frame = m_frames_in_flight[m_current_frame];

        vk::Result fence_wait_result = m_vk_device.waitForFences(frame.in_flight_fence, true, UINT64_MAX);
        if (fence_wait_result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed waiting for frame (fences)");
        }

        auto destroyed = std::vector<VertexDataHandle>();
        for (auto &handle_pair : m_vertex_buffer_deletion_queue) {
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
        for (VertexDataHandle v : destroyed) {
            m_vertex_buffer_deletion_queue.erase(v);
        }

        vk::ResultValue<uint32_t> acquire_result
            = m_vk_device.acquireNextImageKHR(m_vk_swapchain, UINT64_MAX, frame.image_available_semaphore, nullptr);
        if (acquire_result.result != vk::Result::eSuccess && acquire_result.result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("Failed to acquire swapchain image");
        }
        uint32_t image_index = acquire_result.value;

        m_vk_device.resetFences({ frame.in_flight_fence });

        frame.command_buffer.reset();

        record_vk_command_buffer(image_index);

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
                  .setPImageIndices(&image_index);

        vk::Result present_result = m_vk_present_queue.presentKHR(present_info);

        if (present_result == vk::Result::eSuboptimalKHR) {
            recreate_swapchain(window);
        }
        else if (present_result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to present frame");
        }

        m_current_frame = (m_current_frame + 1) % c_frames_in_flight;
    }

    Renderer::~Renderer()
    {
#ifdef MVE_ENABLE_VALIDATION_LAYERS
        cleanup_vk_debug_messenger();
#endif
        m_vk_device.waitIdle();

        cleanup_vk_swapchain();

        for (auto buffer : m_vertex_buffers) {
            vmaDestroyBuffer(m_vma_allocator, buffer.second.buffer.vk_handle, buffer.second.buffer.vma_allocation);
        }
        vmaDestroyAllocator(m_vma_allocator);

        m_vk_device.destroy(m_vk_graphics_pipeline);
        m_vk_device.destroy(m_vk_pipeline_layout);
        m_vk_device.destroy(m_vk_render_pass);

        for (FrameInFlight &frame : m_frames_in_flight) {
            m_vk_device.destroy(frame.render_finished_semaphore);
            m_vk_device.destroy(frame.image_available_semaphore);
            m_vk_device.destroy(frame.in_flight_fence);
        }

        m_vk_device.destroy(m_vk_command_pool);

        m_vk_device.destroy();

        m_vk_instance.destroy(m_vk_surface);
        m_vk_instance.destroy();
    }

    void Renderer::recreate_swapchain(const Window &window)
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

        m_vk_swapchain_extent
            = get_vk_swapchain_extent(swapchain_support_details.capabilities, window.get_glfw_handle());

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

    std::vector<const char *> Renderer::get_vk_instance_required_exts()
    {
        uint32_t glfw_ext_count = 0;
        const char **glfw_exts;
        glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

        std::vector<const char *> exts(glfw_exts, glfw_exts + glfw_ext_count);

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

        auto func
            = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

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
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
        void *user_data_ptr)
    {
        if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            LOG->warn("[Vulkan Debug] " + std::string(callback_data->pMessage));
        }

        return false;
    }

    void Renderer::cleanup_vk_debug_messenger()
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            m_vk_instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr) {
            func(m_vk_instance, m_vk_debug_utils_messenger, nullptr);
        }
    }

    void Renderer::record_vk_command_buffer(uint32_t image_index)
    {
        vk::CommandBuffer &command_buffer = m_frames_in_flight[m_current_frame].command_buffer;

        auto buffer_begin_info = vk::CommandBufferBeginInfo();
        command_buffer.begin(buffer_begin_info);

        auto clear_color = vk::ClearValue(vk::ClearColorValue(std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f }));

        auto render_pass_begin_info
            = vk::RenderPassBeginInfo()
                  .setRenderPass(m_vk_render_pass)
                  .setFramebuffer(m_vk_swapchain_framebuffers[image_index])
                  .setRenderArea(vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_vk_swapchain_extent))
                  .setClearValueCount(1)
                  .setPClearValues(&clear_color);

        command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_vk_graphics_pipeline);

        auto viewport
            = vk::Viewport()
                  .setX(0.0f)
                  .setY(0.0f)
                  .setWidth(static_cast<float>(m_vk_swapchain_extent.width))
                  .setHeight(static_cast<float>(m_vk_swapchain_extent.height))
                  .setMinDepth(0.0f)
                  .setMaxDepth(1.0f);

        command_buffer.setViewport(0, { viewport });

        auto scissor = vk::Rect2D().setOffset({ 0, 0 }).setExtent(m_vk_swapchain_extent);

        command_buffer.setScissor(0, { scissor });

        for (auto vertex_data_pair : m_vertex_buffers) {
            if (m_vertex_buffer_deletion_queue.contains(vertex_data_pair.first)) {
                continue;
            }
            command_buffer.bindVertexBuffers(0, vertex_data_pair.second.buffer.vk_handle, { 0 });
            command_buffer.draw(vertex_data_pair.second.vertex_count, 1, 0, 0);
        }

        command_buffer.endRenderPass();

        command_buffer.end();
    }

    Renderer::VertexBuffer Renderer::create_vertex_buffer(VmaAllocator allocator, const VertexData &vertex_data)
    {
        Buffer buffer = create_buffer(
            allocator,
            get_vertex_layout_bytes(vertex_data.get_layout()) * vertex_data.get_vertex_count(),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU);

        void *data;
        vmaMapMemory(allocator, buffer.vma_allocation, &data);
        memcpy(
            data,
            vertex_data.get_data_ptr(),
            get_vertex_layout_bytes(vertex_data.get_layout()) * vertex_data.get_vertex_count());
        vmaUnmapMemory(allocator, buffer.vma_allocation);

        return { buffer, vertex_data.get_vertex_count() };
    }

    Renderer::VertexDataHandle Renderer::upload_vertex_data(const VertexData &vertex_data)
    {
        VertexBuffer vertex_buffer = create_vertex_buffer(m_vma_allocator, vertex_data);
        m_vertex_buffers[m_vertex_data_handle_count] = vertex_buffer;
        m_vertex_data_handle_count++;

        return m_vertex_data_handle_count - 1;
    }

    vk::VertexInputBindingDescription Renderer::create_vk_binding_description(const VertexLayout &layout)
    {
        auto binding_description
            = vk::VertexInputBindingDescription()
                  .setBinding(0)
                  .setStride(get_vertex_layout_bytes(layout))
                  .setInputRate(vk::VertexInputRate::eVertex);
        return binding_description;
    }

    std::vector<vk::VertexInputAttributeDescription> Renderer::create_vk_attribute_descriptions(
        const VertexLayout &layout)
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
            frames_in_flight.push_back(frame);
        }

        return frames_in_flight;
    }

    void Renderer::queue_destroy(Renderer::VertexDataHandle handle)
    {
        m_vertex_buffer_deletion_queue[handle] = 0;
    }

    Renderer::Buffer Renderer::create_buffer(
        VmaAllocator allocator, size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage)
    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage;

        VmaAllocationCreateInfo vma_alloc_info = {};
        vma_alloc_info.usage = memory_usage;

        VkBuffer vk_buffer;
        VmaAllocation allocation;

        vmaCreateBuffer(allocator, &buffer_info, &vma_alloc_info, &vk_buffer, &allocation, nullptr);

        return { vk_buffer, allocation };
    };

    Shader::Shader(const std::filesystem::path &file_path, ShaderType shader_type)
    {
        LOG->debug("Loading shader: " + file_path.string());

        auto file = std::ifstream(file_path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + file_path.string());
        }

        size_t file_size = (size_t)file.tellg();
        auto buffer = std::vector<char>(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();

        m_spv_code = buffer;
    };

    std::vector<char> Shader::get_spv_code() const
    {
        return m_spv_code;
    }
}
