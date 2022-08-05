#include "app.hpp"

#include "mve.hpp"

#include <spdlog/spdlog.h>

void drawFrame()
{

}

void mainLoop(const mve::UniqueGlfwWindow& window)
{
    while (!glfwWindowShouldClose(window.get()))
    {
        glfwPollEvents();
        drawFrame();
    }
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

        spdlog::debug("Entering main loop");
        mainLoop(window);
    }
}
