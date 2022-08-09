#include "mve.hpp"

#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

#include <shaderc/shaderc.hpp>

#include <filesystem>
#include <fstream>
#include <optional>
#include <set>

#include "logger.hpp"

struct SwapchainSupportDetails
{
    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<vk::SurfaceFormatKHR> surfaceFormats;
    std::vector<vk::PresentModeKHR> surfacePresentModes;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

SwapchainSupportDetails getSwapchainSupportDetails(
    const vk::PhysicalDevice& physicalDevice, const vk::UniqueSurfaceKHR& surface)
{
    SwapchainSupportDetails details {};
    details.surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface.get());
    details.surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface.get());
    details.surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(surface.get());

    return details;
}

std::vector<const char*> getValidationLayers() { return { "VK_LAYER_KHRONOS_validation" }; }

std::vector<const char*> getRequiredDeviceExtensions() { return { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; }

bool hasValidationLayerSupport()
{
    const std::vector<const char*> validationLayers = getValidationLayers();

    const std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    for (const std::string& validationLayer : validationLayers)
    {
        bool layerFound = false;
        for (const vk::LayerProperties& layerProperties : availableLayers)
        {
            if (validationLayer == layerProperties.layerName)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
        {
            return false;
        }
    }
    return true;
}

QueueFamilyIndices getQueueFamilyIndices(const vk::PhysicalDevice& device, const vk::UniqueSurfaceKHR& surface)
{
    QueueFamilyIndices indices {};

    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const vk::QueueFamilyProperties& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }
        if (device.getSurfaceSupportKHR(i, surface.get()))
        {
            indices.presentFamily = i;
        }
        i++;
    }
    return indices;
}

bool hasExtensionSupport(vk::PhysicalDevice physicalDevice)
{
    std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

    std::vector<const char*> requiredExtensions = getRequiredDeviceExtensions();

    for (const std::string& requiredExtension : requiredExtensions)
    {
        bool found = false;
        for (const vk::ExtensionProperties& extensionProperties : availableExtensions)
        {
            if (requiredExtension == extensionProperties.extensionName)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return false;
        }
    }
    return true;
}

vk::PresentModeKHR getBestSwapchainPresentMode(
    const vk::PhysicalDevice& physicalDevice, const vk::UniqueSurfaceKHR& surface)
{
    SwapchainSupportDetails swapchainSupportDetails = getSwapchainSupportDetails(physicalDevice, surface);
    std::vector<vk::PresentModeKHR> availablePresentModes = swapchainSupportDetails.surfacePresentModes;
    for (const vk::PresentModeKHR& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

bool isDeviceSuitable(const vk::PhysicalDevice& physicalDevice, const vk::UniqueSurfaceKHR& surface)
{
    QueueFamilyIndices indices = getQueueFamilyIndices(physicalDevice, surface);

    bool isExtensionsSupported = hasExtensionSupport(physicalDevice);

    bool isSwapchainSupported = false;
    if (isExtensionsSupported)
    {
        SwapchainSupportDetails swapchainSupportDetails = getSwapchainSupportDetails(physicalDevice, surface);
        isSwapchainSupported
            = !swapchainSupportDetails.surfaceFormats.empty() && !swapchainSupportDetails.surfacePresentModes.empty();
    }

    return indices.isComplete() && isSwapchainSupported;
}

namespace mve
{
    UniqueGlfwWindow createWindow(int width, int height, const std::string& title)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        auto windowDeleter = [](GLFWwindow* window)
        {
            glfwDestroyWindow(window);
            glfwTerminate();
        };

        UniqueGlfwWindow window(glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr), windowDeleter);
        return window;
    }

    vk::UniqueInstance createInstance(const std::string& applicationName, uint32_t applicationVersion)
    {

#ifdef ENABLE_VALIDATION_LAYERS
        if (!hasValidationLayerSupport())
        {
            throw std::runtime_error("Validation layers requested but not "
                                     "available");
        }
        LOG->info("Vulkan validation layers enabled");
#endif

        auto applicationInfo
            = vk::ApplicationInfo()
                  .setPApplicationName(applicationName.c_str())
                  .setApplicationVersion(applicationVersion)
                  .setPEngineName("Mini Vulkan Engine")
                  .setApiVersion(VK_MAKE_VERSION(1, 0, 0))
                  .setApiVersion(VK_API_VERSION_1_0);

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwInit();
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

#ifdef ENABLE_VALIDATION_LAYERS
        std::vector<const char*> validationLayers = getValidationLayers();
        auto instanceCreateInfo
            = vk::InstanceCreateInfo()
                  .setFlags(vk::InstanceCreateFlagBits())
                  .setPApplicationInfo(&applicationInfo)
                  .setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()))
                  .setPpEnabledLayerNames(validationLayers.data())
                  .setEnabledExtensionCount(glfwExtensionCount)
                  .setPpEnabledExtensionNames(glfwExtensions);
#else
        auto instanceCreateInfo
            = vk::InstanceCreateInfo()
                  .setFlags(vk::InstanceCreateFlags())
                  .setPApplicationInfo(&applicationInfo)
                  .setEnabledLayerCount(0)
                  .setEnabledExtensionCount(glfwExtensionCount)
                  .setPpEnabledExtensionNames(glfwExtensions);
#endif

        auto instance = vk::createInstanceUnique(instanceCreateInfo);
        return instance;
    }

    vk::UniqueSurfaceKHR createSurface(const UniqueGlfwWindow& window, const vk::UniqueInstance& instance)
    {
        VkSurfaceKHR surface;
        VkResult result = glfwCreateWindowSurface(instance.get(), window.get(), nullptr, &surface);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface");
        }
        vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> surfaceDeleter(instance.get());
        return vk::UniqueSurfaceKHR(surface, surfaceDeleter);
    }

    vk::PhysicalDevice getBestPhysicalDevice(const vk::UniqueInstance& instance, const vk::UniqueSurfaceKHR& surface)
    {
        const std::vector<vk::PhysicalDevice> devices = instance->enumeratePhysicalDevices();
        if (devices.empty())
        {
            throw std::runtime_error("Failed to find GPUs with Vulkan support");
        }

        for (const vk::PhysicalDevice& device : devices)
        {
            if (isDeviceSuitable(device, surface))
            {
                LOG->info("Using Vulkan device: {}", device.getProperties().deviceName);
                return device;
            }
        }
        throw std::runtime_error("Failed to find suitable GPU");
    }

    vk::UniqueDevice createDevice(const vk::UniqueSurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice)
    {
        QueueFamilyIndices indices = ::getQueueFamilyIndices(physicalDevice, surface);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;

        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            auto queueCreateInfo
                = vk::DeviceQueueCreateInfo()
                      .setFlags(vk::DeviceQueueCreateFlags())
                      .setQueueFamilyIndex(queueFamily)
                      .setQueueCount(1)
                      .setPQueuePriorities(&queuePriority);
            queueCreateInfos.push_back(queueCreateInfo);
        }

        vk::PhysicalDeviceFeatures deviceFeatures {};

        std::vector<const char*> requiredExtensions = getRequiredDeviceExtensions();

#ifdef ENABLE_VALIDATION_LAYERS
        std::vector<const char*> validationLayers = getValidationLayers();
        auto deviceCreateInfo
            = vk::DeviceCreateInfo()
                  .setFlags(vk::DeviceCreateFlags())
                  .setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
                  .setPQueueCreateInfos(queueCreateInfos.data())
                  .setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()))
                  .setPpEnabledLayerNames(validationLayers.data())
                  .setEnabledExtensionCount(static_cast<uint32_t>(requiredExtensions.size()))
                  .setPpEnabledExtensionNames(requiredExtensions.data())
                  .setPEnabledFeatures(&deviceFeatures);
#else
        auto deviceCreateInfo
            = vk::DeviceCreateInfo()
                  .setFlags(vk::DeviceCreateFlags())
                  .setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
                  .setPQueueCreateInfos(queueCreateInfos.data())
                  .setEnabledLayerCount(0)
                  .setEnabledExtensionCount(static_cast<uint32_t>(requiredExtensions.size()))
                  .setPpEnabledExtensionNames(requiredExtensions.data())
                  .setPEnabledFeatures(&deviceFeatures);
#endif

        return physicalDevice.createDeviceUnique(deviceCreateInfo);
    }

    vk::Queue getGraphicsQueue(
        const vk::UniqueSurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device)
    {
        QueueFamilyIndices indices = getQueueFamilyIndices(physicalDevice, surface);
        return device->getQueue(indices.graphicsFamily.value(), 0);
    }

    vk::Queue getPresentQueue(
        const vk::UniqueSurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device)
    {
        QueueFamilyIndices indices = getQueueFamilyIndices(physicalDevice, surface);
        return device->getQueue(indices.presentFamily.value(), 0);
    }

    vk::SurfaceFormatKHR getBestSurfaceFormat(
        const vk::UniqueSurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice)
    {
        SwapchainSupportDetails swapchainSupportDetails = getSwapchainSupportDetails(physicalDevice, surface);
        std::vector<vk::SurfaceFormatKHR> availableFormats = swapchainSupportDetails.surfaceFormats;
        for (const vk::SurfaceFormatKHR& availableFormat : availableFormats)
        {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb
                && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    vk::Extent2D getBestExtent(
        const UniqueGlfwWindow& window, const vk::UniqueSurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice)
    {
        SwapchainSupportDetails swapchainSupportDetails = getSwapchainSupportDetails(physicalDevice, surface);
        vk::SurfaceCapabilitiesKHR capabilities = swapchainSupportDetails.surfaceCapabilities;
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width;
            int height;
            glfwGetFramebufferSize(window.get(), &width, &height);

            vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

            actualExtent.width
                = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(
                actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }

    vk::UniqueSwapchainKHR createSwapchain(
        const vk::UniqueSurfaceKHR& surface,
        const vk::PhysicalDevice& physicalDevice,
        const vk::UniqueDevice& device,
        const vk::SurfaceFormatKHR& format,
        const vk::Extent2D& extent)
    {
        SwapchainSupportDetails swapchainSupportDetails = getSwapchainSupportDetails(physicalDevice, surface);

        uint32_t imageCount = swapchainSupportDetails.surfaceCapabilities.minImageCount + 1;

        if (swapchainSupportDetails.surfaceCapabilities.maxImageCount > 0
            && imageCount > swapchainSupportDetails.surfaceCapabilities.maxImageCount)
        {

            imageCount = swapchainSupportDetails.surfaceCapabilities.maxImageCount;
        }

        QueueFamilyIndices indices = getQueueFamilyIndices(physicalDevice, surface);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        vk::SharingMode sharingMode;
        uint32_t queueFamilyIndexCount;
        uint32_t* pQueueFamilyIndices;
        if (indices.graphicsFamily != indices.presentFamily)
        {
            sharingMode = vk::SharingMode::eConcurrent;
            queueFamilyIndexCount = 2;
            pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            sharingMode = vk::SharingMode::eExclusive;
            queueFamilyIndexCount = 0;
            pQueueFamilyIndices = nullptr;
        }
        auto swapchainCreateInfo
            = vk::SwapchainCreateInfoKHR()
                  .setFlags(vk::SwapchainCreateFlagBitsKHR())
                  .setSurface(surface.get())
                  .setMinImageCount(imageCount)
                  .setImageFormat(format.format)
                  .setImageColorSpace(format.colorSpace)
                  .setImageExtent(extent)
                  .setImageArrayLayers(1)
                  .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                  .setImageSharingMode(sharingMode)
                  .setQueueFamilyIndexCount(queueFamilyIndexCount)
                  .setPQueueFamilyIndices(pQueueFamilyIndices)
                  .setPreTransform(swapchainSupportDetails.surfaceCapabilities.currentTransform)
                  .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                  .setPresentMode(getBestSwapchainPresentMode(physicalDevice, surface))
                  .setClipped(true)
                  .setOldSwapchain(nullptr);

        return device->createSwapchainKHRUnique(swapchainCreateInfo);
    }

    std::vector<vk::Image> getSwapchainImages(const vk::UniqueDevice& device, const vk::UniqueSwapchainKHR& swapchain)
    {
        std::vector<vk::Image> images = device->getSwapchainImagesKHR(swapchain.get());
        return images;
    }

    std::vector<vk::UniqueImageView> createImageViews(
        const vk::UniqueDevice& device,
        const vk::SurfaceFormatKHR& surfaceFormat,
        const std::vector<vk::Image>& swapchainImages)
    {
        auto imageViews = std::vector<vk::UniqueImageView>(swapchainImages.size());

        int i = 0;
        for (const vk::Image& image : swapchainImages)
        {
            auto imageViewCreateInfo
                = vk::ImageViewCreateInfo()
                      .setFlags(vk::ImageViewCreateFlags())
                      .setImage(image)
                      .setViewType(vk::ImageViewType::e2D)
                      .setFormat(surfaceFormat.format)
                      .setComponents(vk::ComponentMapping(
                          vk::ComponentSwizzle::eIdentity,
                          vk::ComponentSwizzle::eIdentity,
                          vk::ComponentSwizzle::eIdentity,
                          vk::ComponentSwizzle::eIdentity))
                      .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

            imageViews[i] = device->createImageViewUnique(imageViewCreateInfo);
            i++;
        }
        return imageViews;
    }

    vk::UniqueShaderModule createShaderModule(
        const vk::UniqueDevice& device, const std::filesystem::path& filePath, ShaderType shaderType, bool optimize)
    {
        auto file = std::ifstream(filePath);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open shader file: " + filePath.string());
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        auto compiler = shaderc::Compiler();
        auto options = shaderc::CompileOptions();

        if (optimize)
        {
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        }

        shaderc_shader_kind shaderKind;
        switch (shaderType)
        {
        case ShaderType::eVertex:
            shaderKind = shaderc_glsl_vertex_shader;
            break;
        case ShaderType::eFragment:
            shaderKind = shaderc_glsl_fragment_shader;
            break;
        }

        shaderc::SpvCompilationResult result
            = compiler.CompileGlslToSpv(source, shaderKind, filePath.filename().string().c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            throw std::runtime_error(
                "Failed to compile shader: " + filePath.string() + "\n" + result.GetErrorMessage());
        }

        std::vector<uint32_t> shaderCode = { result.cbegin(), result.cend() };

        auto shaderModuleCreateInfo
            = vk::ShaderModuleCreateInfo()
                  .setFlags(vk::ShaderModuleCreateFlags())
                  .setCodeSize(sizeof(uint32_t) * shaderCode.size())
                  .setPCode(shaderCode.data());
        return device->createShaderModuleUnique(shaderModuleCreateInfo);
    }

    vk::UniquePipeline createGraphicsPipeline(
        const vk::UniqueDevice& device,
        const vk::Extent2D& extent,
        const vk::UniqueShaderModule& vertexShader,
        const vk::UniqueShaderModule& fragmentShader,
        const vk::UniqueRenderPass& renderPass)
    {
        auto vertexShaderStageInfo
            = vk::PipelineShaderStageCreateInfo()
                  .setStage(vk::ShaderStageFlagBits::eVertex)
                  .setModule(vertexShader.get())
                  .setPName("main");
        auto fragmentShaderStageInfo
            = vk::PipelineShaderStageCreateInfo()
                  .setStage(vk::ShaderStageFlagBits::eFragment)
                  .setModule(fragmentShader.get())
                  .setPName("main");

        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

        auto vertexInputInfo
            = vk::PipelineVertexInputStateCreateInfo()
                  .setVertexBindingDescriptionCount(0)
                  .setPVertexBindingDescriptions(nullptr)
                  .setVertexAttributeDescriptionCount(0)
                  .setPVertexAttributeDescriptions(nullptr);

        auto inputAssemblyInfo = vk::PipelineInputAssemblyStateCreateInfo()
                                     .setTopology(vk::PrimitiveTopology::eTriangleList)
                                     .setPrimitiveRestartEnable(false);

        std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

        auto dynamicStateCreateInfo
            = vk::PipelineDynamicStateCreateInfo()
                  .setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
                  .setPDynamicStates(dynamicStates.data());

        auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setScissorCount(1);

        auto rasterizationStateCreateInfo
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

        auto multisampleStateCreateInfo
            = vk::PipelineMultisampleStateCreateInfo()
                  .setSampleShadingEnable(false)
                  .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                  .setMinSampleShading(1.0f)
                  .setPSampleMask(nullptr)
                  .setAlphaToCoverageEnable(false)
                  .setAlphaToOneEnable(false);

        auto colorBlendAttachmentState
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

        auto colorBlendStateCreateInfo
            = vk::PipelineColorBlendStateCreateInfo()
                  .setLogicOpEnable(false)
                  .setLogicOp(vk::LogicOp::eCopy)
                  .setAttachmentCount(1)
                  .setPAttachments(&colorBlendAttachmentState)
                  .setBlendConstants(std::array<float, 4> { 0.0f, 0.0f, 0.0f, 0.0f });

        auto pipelineLayoutCreateInfo
            = vk::PipelineLayoutCreateInfo()
                  .setSetLayoutCount(0)
                  .setPSetLayouts(nullptr)
                  .setPushConstantRangeCount(0)
                  .setPPushConstantRanges(nullptr);

        vk::UniquePipelineLayout pipelineLayout = device->createPipelineLayoutUnique(pipelineLayoutCreateInfo);

        auto pipelineInfo
            = vk::GraphicsPipelineCreateInfo()
                  .setStageCount(2)
                  .setPStages(shaderStages)
                  .setPVertexInputState(&vertexInputInfo)
                  .setPInputAssemblyState(&inputAssemblyInfo)
                  .setPViewportState(&viewportStateCreateInfo)
                  .setPRasterizationState(&rasterizationStateCreateInfo)
                  .setPMultisampleState(&multisampleStateCreateInfo)
                  .setPDepthStencilState(nullptr)
                  .setPColorBlendState(&colorBlendStateCreateInfo)
                  .setPDynamicState(&dynamicStateCreateInfo)
                  .setLayout(pipelineLayout.get())
                  .setRenderPass(renderPass.get())
                  .setSubpass(0)
                  .setBasePipelineHandle(nullptr)
                  .setBasePipelineIndex(-1);

        auto pipelineCreationResult = device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
        if (pipelineCreationResult.result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to create graphics pipeline");
        }
        vk::UniquePipeline graphicsPipeline = std::move(pipelineCreationResult.value);
        return graphicsPipeline;
    }

    vk::UniqueRenderPass createRenderPass(const vk::UniqueDevice& device, const vk::SurfaceFormatKHR& swapchainFormat)
    {
        auto colorAttachment
            = vk::AttachmentDescription()
                  .setFormat(swapchainFormat.format)
                  .setSamples(vk::SampleCountFlagBits::e1)
                  .setLoadOp(vk::AttachmentLoadOp::eClear)
                  .setStoreOp(vk::AttachmentStoreOp::eStore)
                  .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .setInitialLayout(vk::ImageLayout::eUndefined)
                  .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        auto colorAttachmentRef
            = vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eAttachmentOptimal);

        auto subpass = vk::SubpassDescription()
                           .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                           .setColorAttachmentCount(1)
                           .setPColorAttachments(&colorAttachmentRef);

        auto dependency
            = vk::SubpassDependency()
                  .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                  .setDstSubpass(0)
                  .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                  .setSrcAccessMask(vk::AccessFlagBits::eNone)
                  .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                  .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        auto renderPassInfo
            = vk::RenderPassCreateInfo()
                  .setAttachmentCount(1)
                  .setPAttachments(&colorAttachment)
                  .setSubpassCount(1)
                  .setPSubpasses(&subpass)
                  .setDependencyCount(1)
                  .setPDependencies(&dependency);

        vk::UniqueRenderPass renderPass = device->createRenderPassUnique(renderPassInfo);
        return renderPass;
    }

    std::vector<vk::UniqueFramebuffer> createFramebuffers(
        const vk::UniqueDevice& device,
        const vk::Extent2D& extent,
        const std::vector<vk::UniqueImageView>& imageViews,
        const vk::UniqueRenderPass& renderPass)
    {
        auto framebuffers = std::vector<vk::UniqueFramebuffer>(imageViews.size());

        for (size_t i = 0; i < imageViews.size(); i++)
        {
            auto framebufferInfo
                = vk::FramebufferCreateInfo()
                      .setRenderPass(renderPass.get())
                      .setAttachmentCount(1)
                      .setPAttachments(&imageViews[i].get())
                      .setWidth(extent.width)
                      .setHeight(extent.height)
                      .setLayers(1);

            framebuffers[i] = device->createFramebufferUnique(framebufferInfo);
        }
        return framebuffers;
    }

    vk::UniqueCommandPool createCommandPool(
        const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device, const vk::UniqueSurfaceKHR& surface)
    {
        QueueFamilyIndices indices = ::getQueueFamilyIndices(physicalDevice, surface);

        auto commandPoolInfo = vk::CommandPoolCreateInfo()
                                   .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                                   .setQueueFamilyIndex(indices.graphicsFamily.value());

        vk::UniqueCommandPool commandPool = device->createCommandPoolUnique(commandPoolInfo);
        return commandPool;
    }

    void recordCommandBuffer(
        const vk::Extent2D& extent,
        const vk::UniqueRenderPass& renderPass,
        const vk::UniquePipeline& graphicsPipeline,
        const std::vector<vk::UniqueFramebuffer>& framebuffers,
        const vk::UniqueCommandBuffer& commandBuffer,
        uint32_t imageIndex)
    {
        auto beginInfo = vk::CommandBufferBeginInfo();

        commandBuffer->begin(beginInfo);

        auto clearColor = vk::ClearValue(vk::ClearColorValue(std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f }));

        auto renderPassBeginInfo
            = vk::RenderPassBeginInfo()
                  .setRenderPass(renderPass.get())
                  .setFramebuffer(framebuffers[imageIndex].get())
                  .setRenderArea(vk::Rect2D().setOffset(vk::Offset2D(0, 0)).setExtent(extent))
                  .setClearValueCount(1)
                  .setPClearValues(&clearColor);

        commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline.get());

        auto viewport
            = vk::Viewport()
                  .setX(0.0f)
                  .setY(0.0f)
                  .setWidth(static_cast<float>(extent.width))
                  .setHeight(static_cast<float>(extent.height))
                  .setMinDepth(0.0f)
                  .setMaxDepth(1.0f);

        commandBuffer->setViewport(0, 1, &viewport);

        auto scissorRect = vk::Rect2D().setOffset(vk::Offset2D(0, 0)).setExtent(extent);

        commandBuffer->setScissor(0, 1, &scissorRect);

        commandBuffer->draw(3, 1, 0, 0);

        commandBuffer->endRenderPass();

        commandBuffer->end();
    }
    std::vector<vk::UniqueSemaphore> createSemaphores(const vk::UniqueDevice& device, int count)
    {
        auto semaphores = std::vector<vk::UniqueSemaphore>(count);
        for (int i = 0; i < count; i++)
        {
            semaphores[i] = device->createSemaphoreUnique(vk::SemaphoreCreateInfo());
        }
        return semaphores;
    }

    std::vector<vk::UniqueFence> createFences(const vk::UniqueDevice& device, bool signaled, int count)
    {
        auto fences = std::vector<vk::UniqueFence>(count);
        for (int i = 0; i < count; i++)
        {
            auto fenceInfo = vk::FenceCreateInfo();
            if (signaled)
            {
                fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
            }
            fences[i] = device->createFenceUnique(fenceInfo);
        }
        return fences;
    }
    std::vector<vk::UniqueCommandBuffer> createCommandBuffers(
        const vk::UniqueDevice& device, const vk::UniqueCommandPool& commandPool, int count)
    {
        auto commandBufferAllocateInfo
            = vk::CommandBufferAllocateInfo()
                  .setCommandPool(commandPool.get())
                  .setLevel(vk::CommandBufferLevel::ePrimary)
                  .setCommandBufferCount(static_cast<uint32_t>(count));

        std::vector<vk::UniqueCommandBuffer> commandBuffers
            = device->allocateCommandBuffersUnique(commandBufferAllocateInfo);

        return commandBuffers;
    }
}
