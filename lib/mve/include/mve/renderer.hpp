#pragma once

#include <array>

#include <mve/detail/resources.hpp>

namespace mve {

class Window;
class ShaderDescriptorSet;

enum class DeviceType { first, integrated_gpu, discrete_gpu, virtual_gpu, cpu };
enum class TextureFormat { r, rg, rgb, rgba };
enum class Msaa { samples_1, samples_2, samples_4, samples_8, samples_16, samples_32, samples_64 };

struct Version {
    int major;
    int minor;
    int patch;
};

class Renderer {
public:
    Renderer(
        const Window& window,
        const std::string& app_name,
        Version version,
        DeviceType preferred_device_type = DeviceType::first);

    ~Renderer();

    void begin_frame(const Window& window);

    void begin_render_pass_present(const std::optional<std::array<float, 4>>& clear_color = std::nullopt) const;

    void begin_render_pass_framebuffer(
        const Framebuffer& framebuffer, const std::optional<std::array<float, 4>>& clear_color = std::nullopt) const;

    void draw_vertex_buffer(const VertexBuffer& vertex_buffer);

    void bind_vertex_buffer(const VertexBuffer& vertex_buffer) const;

    void bind_graphics_pipeline(const GraphicsPipeline& graphics_pipeline);

    void draw_index_buffer(const IndexBuffer& index_buffer);

    void end_frame(const Window& window);

    void end_render_pass() const;

    void resize(const Window& window);

    VertexBuffer create_vertex_buffer(const VertexData& vertex_data);

    IndexBuffer create_index_buffer(std::span<const uint32_t> indices);

    DescriptorSet create_descriptor_set(
        const GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set);

    UniformBuffer create_uniform_buffer(const ShaderDescriptorBinding& descriptor_binding);

    void write_descriptor_binding(
        DescriptorSet& descriptor_set,
        const ShaderDescriptorBinding& descriptor_binding,
        const UniformBuffer& uniform_buffer);

    void write_descriptor_binding(
        DescriptorSet& descriptor_set, const ShaderDescriptorBinding& descriptor_binding, const Texture& texture);

    Texture create_texture(const std::filesystem::path& path);

    Texture create_texture(TextureFormat format, uint32_t width, uint32_t height, const std::byte* data);

    GraphicsPipeline create_graphics_pipeline(
        const Shader& vertex_shader,
        const Shader& fragment_shader,
        std::span<const VertexAttributeType> vertex_layout,
        bool depth_test = true);

    void destroy(VertexBuffer& vertex_buffer);

    void destroy(IndexBuffer& index_buffer);

    void destroy(GraphicsPipeline& graphics_pipeline);

    void destroy(DescriptorSet& descriptor_set);

    void destroy(UniformBuffer& uniform_buffer);

    void destroy(Texture& texture);

    void destroy(Framebuffer& framebuffer);

    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, float value, bool persist = true);

    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, nnm::Vector2f value, bool persist = true);

    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, nnm::Vector3f value, bool persist = true);

    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, nnm::Vector4f value, bool persist = true);

    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, const nnm::Matrix3f& value, bool persist = true);

    void update_uniform(
        UniformBuffer& uniform_buffer, UniformLocation location, const nnm::Matrix4f& value, bool persist = true);

    void update_uniform(UniformBuffer& uniform_buffer, UniformLocation location, bool value, bool persist = true);

    void bind_descriptor_set(const DescriptorSet& descriptor_set) const;

    void bind_descriptor_sets(const DescriptorSet& descriptor_set_1, const DescriptorSet& descriptor_set_2) const;

    void bind_descriptor_sets(
        const DescriptorSet& descriptor_set_1,
        const DescriptorSet& descriptor_set_2,
        const DescriptorSet& descriptor_set_3) const;

    void bind_descriptor_sets(
        const DescriptorSet& descriptor_set_1,
        const DescriptorSet& descriptor_set_2,
        const DescriptorSet& descriptor_set_3,
        const DescriptorSet& descriptor_set_4) const;

    [[nodiscard]] nnm::Vector2i extent() const;

    Framebuffer create_framebuffer();

    void set_framebuffer_resize_callback(Framebuffer& framebuffer, std::function<void()> callback);

    void remove_framebuffer_resize_callback(Framebuffer& framebuffer);

    [[nodiscard]] nnm::Vector2i framebuffer_size(const Framebuffer& framebuffer) const;

    const Texture& framebuffer_texture(const Framebuffer& framebuffer);

    [[nodiscard]] std::string gpu_name() const;

    [[nodiscard]] nnm::Vector2i texture_size(const Texture& texture) const;

    [[nodiscard]] Msaa max_msaa_samples() const;

    void set_msaa_samples(const Window& window, Msaa samples);

    [[nodiscard]] Msaa current_msaa_samples() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
}

#include <mve/detail/resources.inl>
