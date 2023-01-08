#pragma once

#include <cstdint>
#include <functional>

namespace mve {

class DescriptorSetHandle {
public:
    DescriptorSetHandle();

    DescriptorSetHandle(uint64_t value);

    void set(uint64_t value);

    [[nodiscard]] uint64_t value() const;

    [[nodiscard]] bool operator==(const DescriptorSetHandle& other) const;

    [[nodiscard]] bool operator<(const DescriptorSetHandle& other) const;

private:
    bool m_initialized = false;
    uint64_t m_value;
};

class Renderer;
class GraphicsPipelineHandle;
class GraphicsPipeline;
class ShaderDescriptorBinding;
class UniformBufferHandle;
class UniformBuffer;
class TextureHandle;
class Texture;
class ShaderDescriptorSet;

class DescriptorSet {
public:
    DescriptorSet(Renderer& renderer, GraphicsPipeline& graphics_pipeline, const ShaderDescriptorSet& descriptor_set);

    DescriptorSet(Renderer& renderer, DescriptorSetHandle handle);

    DescriptorSet(const DescriptorSet&) = delete;

    DescriptorSet(DescriptorSet&& other);

    ~DescriptorSet();

    DescriptorSet& operator=(const DescriptorSet&) = delete;

    DescriptorSet& operator=(DescriptorSet&& other);

    [[nodiscard]] bool operator==(const DescriptorSet& other) const;

    [[nodiscard]] bool operator<(const DescriptorSet& other) const;

    [[nodiscard]] DescriptorSetHandle handle() const;

    [[nodiscard]] bool is_valid() const;

    void write_binding(const ShaderDescriptorBinding& descriptor_binding, UniformBuffer& uniform_buffer);

    void write_binding(const ShaderDescriptorBinding& descriptor_binding, Texture& texture);

private:
    bool m_valid = false;
    Renderer* m_renderer;
    DescriptorSetHandle m_handle;
};
}

namespace std {
template <>
struct hash<mve::DescriptorSetHandle> {
    std::size_t operator()(const mve::DescriptorSetHandle& handle) const
    {
        return hash<uint64_t>()(handle.value());
    }
};

template <>
struct hash<mve::DescriptorSet> {
    std::size_t operator()(const mve::DescriptorSet& descriptor_set) const
    {
        return hash<mve::DescriptorSetHandle>()(descriptor_set.handle());
    }
};
}