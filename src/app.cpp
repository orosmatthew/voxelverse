#include "app.hpp"

#include "mve.hpp"

#include <spdlog/spdlog.h>

void drawFrame(
    const vk::UniqueDevice& device,
    const vk::UniqueFence& inFlightFence,
    const vk::UniqueSwapchainKHR& swapchain,
    const vk::UniqueSemaphore& imageAvailableSemaphore,
    const vk::UniqueCommandBuffer& commandBuffer,
    const vk::Extent2D& extent,
    const vk::UniqueRenderPass& renderPass,
    const vk::UniquePipeline& graphicsPipeline,
    const std::vector<vk::UniqueFramebuffer>& framebuffers,
    const vk::UniqueSemaphore& renderFinishedSemaphore,
    const vk::Queue& graphicsQueue,
    const vk::Queue& presentQueue)
{
    vk::Result fencesWaitResult = device->waitForFences(1, &inFlightFence.get(), true, UINT64_MAX);
    if (fencesWaitResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("Waiting for frame failed");
    }
    vk::Result resetFencesResult = device->resetFences(1, &inFlightFence.get());
    if (resetFencesResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("Resetting fences failed");
    }

    vk::ResultValue<uint32_t> nextImageResult
        = device->acquireNextImageKHR(swapchain.get(), UINT64_MAX, imageAvailableSemaphore.get(), nullptr);
    if (nextImageResult.result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to acquire swapchain image");
    }
    uint32_t imageIndex = nextImageResult.value;

    commandBuffer->reset(vk::CommandBufferResetFlags());

    mve::recordCommandBuffer(extent, renderPass, graphicsPipeline, framebuffers, commandBuffer, imageIndex);

    vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore.get() };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphore.get() };

    auto submitInfo
        = vk::SubmitInfo()
              .setWaitSemaphoreCount(1)
              .setPWaitSemaphores(waitSemaphores)
              .setPWaitDstStageMask(waitStages)
              .setCommandBufferCount(1)
              .setPCommandBuffers(&commandBuffer.get())
              .setSignalSemaphoreCount(1)
              .setPSignalSemaphores(signalSemaphores);

    if (graphicsQueue.submit(1, &submitInfo, inFlightFence.get()) != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to submit draw command");
    }

    vk::SwapchainKHR swapchains[] = { swapchain.get() };

    auto presentInfo
        = vk::PresentInfoKHR()
              .setWaitSemaphoreCount(1)
              .setPWaitSemaphores(signalSemaphores)
              .setSwapchainCount(1)
              .setPSwapchains(swapchains)
              .setPImageIndices(&imageIndex);

    if (presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to present");
    }
}

void mainLoop(
    const mve::UniqueGlfwWindow& window,
    const vk::UniqueDevice& device,
    const vk::UniqueFence& inFlightFence,
    const vk::UniqueSwapchainKHR& swapchain,
    const vk::UniqueSemaphore& imageAvailableSemaphore,
    const vk::UniqueCommandBuffer& commandBuffer,
    const vk::Extent2D& extent,
    const vk::UniqueRenderPass& renderPass,
    const vk::UniquePipeline& graphicsPipeline,
    const std::vector<vk::UniqueFramebuffer>& framebuffers,
    const vk::UniqueSemaphore& renderFinishedSemaphore,
    const vk::Queue& graphicsQueue,
    const vk::Queue& presentQueue)
{
    while (!glfwWindowShouldClose(window.get()))
    {
        glfwPollEvents();
        drawFrame(
            device,
            inFlightFence,
            swapchain,
            imageAvailableSemaphore,
            commandBuffer,
            extent,
            renderPass,
            graphicsPipeline,
            framebuffers,
            renderFinishedSemaphore,
            graphicsQueue,
            presentQueue);
    }
    spdlog::info("Exiting");
    device->waitIdle();
}

namespace app
{
    void run()
    {
        spdlog::debug("Creating window");
        mve::UniqueGlfwWindow window = mve::createWindow(800, 600, "Mini Vulkan Engine");

        spdlog::debug("Creating Vulkan instance");
        vk::UniqueInstance instance = mve::createInstance("Hello Vulkan", 1);

        spdlog::debug("Creating Vulkan surface");
        vk::UniqueSurfaceKHR surface = mve::createSurface(window, instance);

        spdlog::debug("Getting Vulkan physical device");
        vk::PhysicalDevice physicalDevice = mve::getBestPhysicalDevice(instance, surface);

        spdlog::debug("Creating Vulkan logical device");
        vk::UniqueDevice device = mve::createDevice(surface, physicalDevice);

        spdlog::debug("Getting Vulkan graphics Queue");
        vk::Queue graphicsQueue = mve::getGraphicsQueue(surface, physicalDevice, device);

        spdlog::debug("Getting Vulkan present Queue");
        vk::Queue presentQueue = mve::getPresentQueue(surface, physicalDevice, device);

        spdlog::debug("Getting bet Vulkan surface format");
        vk::SurfaceFormatKHR surfaceFormat = mve::getBestSurfaceFormat(surface, physicalDevice);

        spdlog::debug("Getting best Vulkan extent");
        vk::Extent2D extent = mve::getBestExtent(window, surface, physicalDevice);

        spdlog::debug("Creating Vulkan swapchain");
        vk::UniqueSwapchainKHR swapchain = mve::createSwapchain(surface, physicalDevice, device, surfaceFormat, extent);

        spdlog::debug("Getting Vulkan swapchain Images");
        std::vector<vk::Image> swapchainImages = mve::getSwapchainImages(device, swapchain);

        spdlog::debug("Getting Vulkan image views");
        std::vector<vk::UniqueImageView> imageViews = mve::createImageViews(device, surfaceFormat, swapchainImages);

        spdlog::debug("Creating Vulkan vertex shader module");
        vk::UniqueShaderModule vertexShaderModule = mve::createShaderModule(
            device, "C:/dev/vulkan_testing/shaders/simple.vert", mve::ShaderType::eVertex, true);

        spdlog::debug("Creating Vulkan fragment shader module");
        vk::UniqueShaderModule fragmentShaderModule = mve::createShaderModule(
            device, "C:/dev/vulkan_testing/shaders/simple.frag", mve::ShaderType::eFragment, true);

        spdlog::debug("Creating Vulkan render pass");
        vk::UniqueRenderPass renderPass = mve::createRenderPass(device, surfaceFormat);

        spdlog::debug("Creating Vulkan graphics pipeline");
        vk::UniquePipeline graphicsPipeline
            = mve::createGraphicsPipeline(device, extent, vertexShaderModule, fragmentShaderModule, renderPass);

        spdlog::debug("Creating Vulkan frame buffers");
        std::vector<vk::UniqueFramebuffer> framebuffers
            = mve::createFramebuffers(device, extent, imageViews, renderPass);

        spdlog::debug("Creating Vulkan command pool");
        vk::UniqueCommandPool commandPool = mve::createCommandPool(physicalDevice, device, surface);

        spdlog::debug("Creating Vulkan command buffer");
        vk::UniqueCommandBuffer commandBuffer = mve::createCommandBuffer(device, commandPool);

        spdlog::debug("Creating sync objects");
        vk::UniqueSemaphore imageAvailableSemaphore = mve::createSemaphore(device);
        vk::UniqueSemaphore renderFinishedSemaphore = mve::createSemaphore(device);
        vk::UniqueFence inFlightFence = mve::createFence(device, true);

        spdlog::debug("Entering main loop");
        mainLoop(
            window,
            device,
            inFlightFence,
            swapchain,
            imageAvailableSemaphore,
            commandBuffer,
            extent,
            renderPass,
            graphicsPipeline,
            framebuffers,
            renderFinishedSemaphore,
            graphicsQueue,
            presentQueue);
    }
}
