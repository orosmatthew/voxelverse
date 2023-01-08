#pragma once

#include <cstdint>
#include <functional>

namespace mve {

class DescriptorSetHandle {
public:
    DescriptorSetHandle();

    DescriptorSetHandle(uint32_t value);

    void set(uint32_t value);

    [[nodiscard]] uint32_t value() const;

    [[nodiscard]] bool operator==(const DescriptorSetHandle& other) const;

    [[nodiscard]] bool operator<(const DescriptorSetHandle& other) const;

private:
    bool m_initialized = false;
    uint32_t m_value;
};

class Renderer;
class GraphicsPipelineHandle;
class GraphicsPipeline;
class ShaderDescriptorBinding;

class DescriptorSet {
public:
    DescriptorSet(Renderer& renderer, GraphicsPipeline& pipeline, uint32_t set);
    DescriptorSet(Renderer& renderer, GraphicsPipelineHandle pipeline, uint32_t set);

    DescriptorSet(const DescriptorSet&) = delete;

    DescriptorSet(DescriptorSet&& other);

    ~DescriptorSet();

    DescriptorSet& operator=(const DescriptorSet&) = delete;

    DescriptorSet& operator=(DescriptorSet&& other);

    [[nodiscard]] bool operator==(const DescriptorSet& other) const;

    [[nodiscard]] bool operator<(const DescriptorSet& other) const;

    [[nodiscard]] DescriptorSetHandle handle() const;

    [[nodiscard]] bool is_valid() const;

    // TODO: also make one with UniformBuffer class
    //    void write_binding(const ShaderDescriptorBinding& binding, UniformBufferHandle uniform_buffer);

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
        return hash<uint32_t>()(handle.value());
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