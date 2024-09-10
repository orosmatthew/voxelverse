#pragma once

#include <variant>

namespace mve::detail {

static constexpr size_t max_uniform_value_size = std::max(
    { sizeof(float),
      sizeof(nnm::Vector2f),
      sizeof(nnm::Vector3f),
      sizeof(nnm::Vector4f),
      sizeof(nnm::Matrix3f),
      sizeof(nnm::Matrix4f) });

enum class DescriptorBindingType { uniform_buffer, texture };

struct Image {
    vk::Image vk_handle;
    VmaAllocation vma_allocation {};
    uint32_t width {};
    uint32_t height {};
};

struct DepthImage {
    Image image;
    vk::ImageView vk_image_view;
};

struct RenderImage {
    Image image;
    vk::ImageView vk_image_view;
};

struct TextureImpl {
    Image image;
    vk::ImageView vk_image_view;
    vk::Sampler vk_sampler;
    uint32_t mip_levels {};
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    [[nodiscard]] bool is_complete() const
    {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
};

struct Buffer {
    vk::Buffer vk_handle;
    VmaAllocation vma_allocation {};
};

struct VertexBufferImpl {
    Buffer buffer;
    int vertex_count {};
};

struct IndexBufferImpl {
    Buffer buffer;
    size_t index_count {};
};

struct UniformBufferImpl {
    Buffer buffer;
    uint32_t size {};
    std::byte* mapped_ptr {};
};

struct DescriptorSetImpl {
    Handle id {};
    vk::DescriptorSet vk_handle;
    vk::DescriptorPool vk_pool;
};

struct FrameInFlight {
    vk::CommandBuffer command_buffer;
    vk::Semaphore image_available_semaphore;
    vk::Semaphore render_finished_semaphore;
    vk::Fence in_flight_fence;
    std::vector<std::optional<UniformBufferImpl>> uniform_buffers {};
    std::vector<std::optional<DescriptorSetImpl>> descriptor_sets {};
};

struct CurrentDrawState {
    bool is_drawing {};
    uint32_t image_index {};
    vk::CommandBuffer command_buffer;
    std::optional<Handle> current_pipeline {};
    uint32_t frame_index {};
};

struct GraphicsPipelineLayoutImpl {
    vk::PipelineLayout vk_handle;
    std::unordered_map<Handle, Handle> descriptor_set_layouts;
};

struct GraphicsPipelineImpl {
    Handle layout {};
    vk::Pipeline pipeline;
    Shader vertex_shader;
    Shader fragment_shader;
    std::vector<VertexAttributeType> vertex_layout;
    bool depth_test;
};

struct FramebufferImpl {
    std::vector<vk::Framebuffer> vk_framebuffers;
    Texture texture;
    std::optional<std::function<void()>> callback;
    nnm::Vector2i size;
};

struct DeferredFunction {
    std::function<void(uint32_t)> function;
    int counter;
};

struct DeferredUniformUpdateData {
    int counter {};
    Handle handle {};
    UniformLocation location;
    std::array<std::byte, max_uniform_value_size> data {};
    size_t data_size {};
};

struct DeferredCopyStagingBuffer {
    Buffer staging_buffer;
    Buffer dst_buffer;
    size_t buffer_size {};
};

struct DeferredDestroyBuffer {
    Buffer buffer;
    uint32_t frame_index {};
};

struct DeferredDescriptorWriteData {
    int counter {};
    DescriptorBindingType data_type {};
    Handle data_handle {};
    Handle descriptor_handle {};
    uint32_t binding {};
};

struct DeferredCopyBufferImage {
    uint32_t mip_levels {};
    Buffer staging_buffer;
    Image dst_image;
    vk::Format image_format {};
};

struct DeferredDestroyVertexBuffer {
    Handle handle {};
    int frame_count = 0;
};

struct DeferredDestroyTexture {
    Handle handle {};
    int frame_count = 0;
};

struct DeferredDestroyDescriptorSet {
    Handle handle {};
    int frame_count = 0;
};

struct DeferredDestroyGraphicsPipeline {
    Handle handle {};
    int frame_count = 0;
};

struct DeferredDestroyUniformBuffer {
    Handle handle {};
    int frame_count = 0;
};

struct DeferredDestroyIndexBuffer {
    Handle handle {};
    int frame_count = 0;
};

struct DeferredDestroyFramebuffer {
    Handle handle {};
    int frame_count = 0;
};

using DeferredOperation = std::variant<
    DeferredUniformUpdateData,
    DeferredCopyStagingBuffer,
    DeferredDestroyBuffer,
    DeferredDescriptorWriteData,
    DeferredCopyBufferImage,
    DeferredDestroyVertexBuffer,
    DeferredDestroyTexture,
    DeferredDestroyDescriptorSet,
    DeferredDestroyGraphicsPipeline,
    DeferredDestroyUniformBuffer,
    DeferredDestroyIndexBuffer,
    DeferredDestroyFramebuffer>;
}