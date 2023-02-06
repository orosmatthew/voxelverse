#pragma once

#include <cstdint>
#include <functional>

namespace mve {

class Renderer;
class GraphicsPipeline;
class ShaderDescriptorBinding;
class UniformBufferHandle;
class UniformBuffer;
class TextureHandle;
class Texture;
class ShaderDescriptorSet;

class DescriptorSet {

    friend Renderer;

public:
    DescriptorSet(Renderer& renderer, GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set);

    DescriptorSet(Renderer& renderer, size_t handle);

    DescriptorSet(const DescriptorSet&) = delete;

    DescriptorSet(DescriptorSet&& other);

    ~DescriptorSet();

    DescriptorSet& operator=(const DescriptorSet&) = delete;

    DescriptorSet& operator=(DescriptorSet&& other);

    [[nodiscard]] bool operator==(const DescriptorSet& other) const;

    [[nodiscard]] bool operator<(const DescriptorSet& other) const;

    [[nodiscard]] size_t handle() const;

    [[nodiscard]] bool is_valid() const;

    void invalidate();

    void write_binding(const ShaderDescriptorBinding& descriptor_binding, UniformBuffer& uniform_buffer);

    void write_binding(const ShaderDescriptorBinding& descriptor_binding, Texture& texture);

private:
    bool m_valid = false;
    Renderer* m_renderer;
    size_t m_handle;
};
}

namespace std {

template <>
struct hash<mve::DescriptorSet> {
    std::size_t operator()(const mve::DescriptorSet& descriptor_set) const
    {
        return hash<uint64_t>()(descriptor_set.handle());
    }
};
}