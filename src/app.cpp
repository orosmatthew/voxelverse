#include "app.hpp"

#include "mve.hpp"

#include "logger.hpp"

struct VulkanState
{
    vk::UniqueInstance instance;
    vk::UniqueSurfaceKHR surface;
    vk::PhysicalDevice physicalDevice;
    vk::UniqueDevice device;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::Extent2D extent;
    vk::UniqueSwapchainKHR swapchain;
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::UniqueImageView> imageViews;
    vk::UniqueShaderModule vertexShaderModule;
    vk::UniqueShaderModule fragmentShaderModule;
    vk::UniqueRenderPass renderPass;
    vk::UniquePipeline graphicsPipeline;
    std::vector<vk::UniqueFramebuffer> framebuffers;
    vk::UniqueCommandPool commandPool;
    std::vector<vk::UniqueCommandBuffer> commandBuffers;
    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
    std::vector<vk::UniqueFence> inFlightFences;
};

void drawFrame(const VulkanState& vulkanState, int drawFrame)
{

    vk::Result fencesWaitResult
        = vulkanState.device->waitForFences(vulkanState.inFlightFences[drawFrame].get(), true, UINT64_MAX);
    if (fencesWaitResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("Waiting for frame failed");
    }
    vk::Result resetFencesResult = vulkanState.device->resetFences(1, &vulkanState.inFlightFences[drawFrame].get());
    if (resetFencesResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("Resetting fences failed");
    }

    vk::ResultValue<uint32_t> nextImageResult = vulkanState.device->acquireNextImageKHR(
        vulkanState.swapchain.get(), UINT64_MAX, vulkanState.imageAvailableSemaphores[drawFrame].get(), nullptr);
    if (nextImageResult.result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to acquire swapchain image");
    }
    uint32_t imageIndex = nextImageResult.value;

    vulkanState.commandBuffers[drawFrame]->reset(vk::CommandBufferResetFlags());

    mve::recordCommandBuffer(
        vulkanState.extent,
        vulkanState.renderPass,
        vulkanState.graphicsPipeline,
        vulkanState.framebuffers,
        vulkanState.commandBuffers[drawFrame],
        imageIndex);

    vk::Semaphore waitSemaphores[] = { vulkanState.imageAvailableSemaphores[drawFrame].get() };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::Semaphore signalSemaphores[] = { vulkanState.renderFinishedSemaphores[drawFrame].get() };

    auto submitInfo
        = vk::SubmitInfo()
              .setWaitSemaphoreCount(1)
              .setPWaitSemaphores(waitSemaphores)
              .setPWaitDstStageMask(waitStages)
              .setCommandBufferCount(1)
              .setPCommandBuffers(&vulkanState.commandBuffers[drawFrame].get())
              .setSignalSemaphoreCount(1)
              .setPSignalSemaphores(signalSemaphores);

    if (vulkanState.graphicsQueue.submit(1, &submitInfo, vulkanState.inFlightFences[drawFrame].get())
        != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to submit draw command");
    }

    vk::SwapchainKHR swapchains[] = { vulkanState.swapchain.get() };

    auto presentInfo
        = vk::PresentInfoKHR()
              .setWaitSemaphoreCount(1)
              .setPWaitSemaphores(signalSemaphores)
              .setSwapchainCount(1)
              .setPSwapchains(swapchains)
              .setPImageIndices(&imageIndex);

    if (vulkanState.presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to present");
    }
}

void mainLoop(const mve::UniqueGlfwWindow& window, const VulkanState& vulkanState)
{
    int currentDrawFrame = 0;
    while (!glfwWindowShouldClose(window.get()))
    {
        glfwPollEvents();
        drawFrame(vulkanState, currentDrawFrame);
        currentDrawFrame = (currentDrawFrame + 1) % MVE_FRAMES_IN_FLIGHT;
    }
    LOG->info("Exiting");
    vulkanState.device->waitIdle();
}

namespace app
{
    void run()
    {
        LOG->debug("Creating window");
        mve::UniqueGlfwWindow window = mve::createWindow(800, 600, "Mini Vulkan Engine");

        LOG->debug("Creating Vulkan instance");
        vk::UniqueInstance instance = mve::createInstance("Hello Vulkan", 1);

        LOG->debug("Creating Vulkan surface");
        vk::UniqueSurfaceKHR surface = mve::createSurface(window, instance);

        LOG->debug("Getting Vulkan physical device");
        vk::PhysicalDevice physicalDevice = mve::getBestPhysicalDevice(instance, surface);

        LOG->debug("Creating Vulkan logical device");
        vk::UniqueDevice device = mve::createDevice(surface, physicalDevice);

        LOG->debug("Getting Vulkan graphics Queue");
        vk::Queue graphicsQueue = mve::getGraphicsQueue(surface, physicalDevice, device);

        LOG->debug("Getting Vulkan present Queue");
        vk::Queue presentQueue = mve::getPresentQueue(surface, physicalDevice, device);

        LOG->debug("Getting bet Vulkan surface format");
        vk::SurfaceFormatKHR surfaceFormat = mve::getBestSurfaceFormat(surface, physicalDevice);

        LOG->debug("Getting best Vulkan extent");
        vk::Extent2D extent = mve::getBestExtent(window, surface, physicalDevice);

        LOG->debug("Creating Vulkan swapchain");
        vk::UniqueSwapchainKHR swapchain = mve::createSwapchain(surface, physicalDevice, device, surfaceFormat, extent);

        LOG->debug("Getting Vulkan swapchain Images");
        std::vector<vk::Image> swapchainImages = mve::getSwapchainImages(device, swapchain);

        LOG->debug("Getting Vulkan image views");
        std::vector<vk::UniqueImageView> imageViews = mve::createImageViews(device, surfaceFormat, swapchainImages);

        LOG->debug("Creating Vulkan vertex shader module");
        vk::UniqueShaderModule vertexShaderModule = mve::createShaderModule(
            device, "C:/dev/vulkan_testing/shaders/simple.vert", mve::ShaderType::eVertex, true);

        LOG->debug("Creating Vulkan fragment shader module");
        vk::UniqueShaderModule fragmentShaderModule = mve::createShaderModule(
            device, "C:/dev/vulkan_testing/shaders/simple.frag", mve::ShaderType::eFragment, true);

        LOG->debug("Creating Vulkan render pass");
        vk::UniqueRenderPass renderPass = mve::createRenderPass(device, surfaceFormat);

        LOG->debug("Creating Vulkan graphics pipeline");
        vk::UniquePipeline graphicsPipeline
            = mve::createGraphicsPipeline(device, extent, vertexShaderModule, fragmentShaderModule, renderPass);

        LOG->debug("Creating Vulkan frame buffers");
        std::vector<vk::UniqueFramebuffer> framebuffers
            = mve::createFramebuffers(device, extent, imageViews, renderPass);

        LOG->debug("Creating Vulkan command pool");
        vk::UniqueCommandPool commandPool = mve::createCommandPool(physicalDevice, device, surface);

        LOG->debug("Creating Vulkan command buffer");
        std::vector<vk::UniqueCommandBuffer> commandBuffers
            = mve::createCommandBuffers(device, commandPool, MVE_FRAMES_IN_FLIGHT);

        LOG->debug("Creating sync objects");
        std::vector<vk::UniqueSemaphore> imageAvailableSemaphores = mve::createSemaphores(device, MVE_FRAMES_IN_FLIGHT);
        std::vector<vk::UniqueSemaphore> renderFinishedSemaphores = mve::createSemaphores(device, MVE_FRAMES_IN_FLIGHT);
        std::vector<vk::UniqueFence> inFlightFences = mve::createFences(device, true, MVE_FRAMES_IN_FLIGHT);

        auto vulkanState = VulkanState {
            std::move(instance),
            std::move(surface),
            physicalDevice,
            std::move(device),
            graphicsQueue,
            presentQueue,
            surfaceFormat,
            extent,
            std::move(swapchain),
            swapchainImages,
            std::move(imageViews),
            std::move(vertexShaderModule),
            std::move(fragmentShaderModule),
            std::move(renderPass),
            std::move(graphicsPipeline),
            std::move(framebuffers),
            std::move(commandPool),
            std::move(commandBuffers),
            std::move(imageAvailableSemaphores),
            std::move(renderFinishedSemaphores),
            std::move(inFlightFences)
        };

        LOG->debug("Entering main loop");
        mainLoop(window, vulkanState);
    }
}
