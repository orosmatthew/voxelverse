#ifndef MVE_HPP
#define MVE_HPP

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <filesystem>

#define MVE_FRAMES_IN_FLIGHT 2

namespace mve
{
    typedef std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>> UniqueGlfwWindow;

    enum class ShaderType
    {
        eVertex,
        eFragment,
    };

    UniqueGlfwWindow createWindow(int width, int height, const std::string& title);

    vk::UniqueInstance createInstance(const std::string& applicationName, uint32_t applicationVersion);

    vk::UniqueSurfaceKHR createSurface(GLFWwindow* window, vk::Instance instance);

    vk::PhysicalDevice getBestPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);

    vk::UniqueDevice createDevice(vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice);

    vk::Queue getGraphicsQueue(vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice, vk::Device device);

    vk::Queue getPresentQueue(vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice, vk::Device device);

    vk::SurfaceFormatKHR getBestSurfaceFormat(vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice);

    vk::Extent2D getBestExtent(GLFWwindow* window, vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice);

    vk::UniqueSwapchainKHR createSwapchain(
        vk::SurfaceKHR surface,
        vk::PhysicalDevice physicalDevice,
        vk::Device device,
        vk::SurfaceFormatKHR format,
        vk::Extent2D extent);

    std::vector<vk::Image> getSwapchainImages(vk::Device device, vk::SwapchainKHR swapchain);

    std::vector<vk::UniqueImageView> createImageViews(
        vk::Device device, vk::SurfaceFormatKHR surfaceFormat, const std::vector<vk::Image>& swapchainImages);

    vk::UniqueShaderModule createShaderModule(
        vk::Device device, const std::filesystem::path& filePath, ShaderType shaderType, bool optimize);

    vk::UniqueRenderPass createRenderPass(vk::Device device, vk::SurfaceFormatKHR swapchainFormat);

    vk::UniquePipeline createGraphicsPipeline(
        vk::Device device,
        vk::Extent2D extent,
        vk::ShaderModule vertexShader,
        vk::ShaderModule fragmentShader,
        vk::RenderPass renderPass);

    std::vector<vk::UniqueFramebuffer> createFramebuffers(
        vk::Device device,
        vk::Extent2D extent,
        const std::vector<vk::UniqueImageView>& imageViews,
        vk::RenderPass renderPass);

    vk::UniqueCommandPool createCommandPool(
        vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);

    std::vector<vk::UniqueCommandBuffer> createCommandBuffers(
        vk::Device device, vk::CommandPool commandPool, int count);

    void recordCommandBuffer(
        vk::Extent2D extent,
        vk::RenderPass renderPass,
        vk::Pipeline graphicsPipeline,
        const std::vector<vk::UniqueFramebuffer>& framebuffers,
        vk::CommandBuffer commandBuffer,
        uint32_t imageIndex);

    std::vector<vk::UniqueSemaphore> createSemaphores(const vk::UniqueDevice& device, int count);

    std::vector<vk::UniqueFence> createFences(const vk::UniqueDevice& device, bool signaled, int count);
}

#endif // MVE_HPP
