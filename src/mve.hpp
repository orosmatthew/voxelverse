#ifndef MVE_HPP
#define MVE_HPP

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <filesystem>

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

    vk::UniqueSurfaceKHR createSurface(const UniqueGlfwWindow& window, const vk::UniqueInstance& instance);

    vk::PhysicalDevice getBestPhysicalDevice(const vk::UniqueInstance& instance, const vk::UniqueSurfaceKHR& surface);

    vk::UniqueDevice createDevice(const vk::UniqueSurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice);

    vk::Queue getGraphicsQueue(
        const vk::UniqueSurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device);

    vk::Queue getPresentQueue(
        const vk::UniqueSurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device);

    vk::SurfaceFormatKHR getBestSurfaceFormat(
        const vk::UniqueSurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice);

    vk::Extent2D getBestExtent(
        const UniqueGlfwWindow& window, const vk::UniqueSurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice);

    vk::UniqueSwapchainKHR createSwapchain(
        const vk::UniqueSurfaceKHR& surface,
        const vk::PhysicalDevice& physicalDevice,
        const vk::UniqueDevice& device,
        const vk::SurfaceFormatKHR& format,
        const vk::Extent2D& extent);

    std::vector<vk::Image> getSwapchainImages(const vk::UniqueDevice& device, const vk::UniqueSwapchainKHR& swapchain);

    std::vector<vk::UniqueImageView> createImageViews(
        const vk::UniqueDevice& device,
        const vk::SurfaceFormatKHR& surfaceFormat,
        const std::vector<vk::Image>& swapchainImages);

    vk::UniqueShaderModule createShaderModule(
        const vk::UniqueDevice& device, const std::filesystem::path& filePath, ShaderType shaderType, bool optimize);

    vk::UniqueRenderPass createRenderPass(const vk::UniqueDevice& device, const vk::SurfaceFormatKHR& swapchainFormat);

    vk::UniquePipeline createGraphicsPipeline(
        const vk::UniqueDevice& device,
        const vk::Extent2D& extent,
        const vk::UniqueShaderModule& vertexShader,
        const vk::UniqueShaderModule& fragmentShader,
        const vk::UniqueRenderPass& renderPass);

    std::vector<vk::UniqueFramebuffer> createFramebuffers(
        const vk::UniqueDevice& device,
        const vk::Extent2D& extent,
        const std::vector<vk::UniqueImageView>& imageViews,
        const vk::UniqueRenderPass& renderPass);

    vk::UniqueCommandPool createCommandPool(
        const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device, const vk::UniqueSurfaceKHR& surface);

    vk::UniqueCommandBuffer createCommandBuffer(
        const vk::UniqueDevice& device, const vk::UniqueCommandPool& commandPool);

    void recordCommandBuffer(
        const vk::Extent2D& extent,
        const vk::UniqueRenderPass& renderPass,
        const vk::UniquePipeline& graphicsPipeline,
        const std::vector<vk::UniqueFramebuffer>& framebuffers,
        const vk::UniqueCommandBuffer& commandBuffer,
        uint32_t imageIndex);

    vk::UniqueSemaphore createSemaphore(const vk::UniqueDevice& device);

    vk::UniqueFence createFence(const vk::UniqueDevice& device, bool signaled);
}

#endif // MVE_HPP
