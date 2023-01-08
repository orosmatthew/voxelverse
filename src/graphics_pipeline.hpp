#pragma once

#include <cstdint>
#include <functional>

#include "vertex_data.hpp"

namespace mve {

class GraphicsPipelineHandle {
public:
    GraphicsPipelineHandle();

    GraphicsPipelineHandle(uint32_t value);

    void set(uint32_t value);

    [[nodiscard]] uint32_t value() const;

    [[nodiscard]] bool operator==(const GraphicsPipelineHandle& other) const;

    [[nodiscard]] bool operator<(const GraphicsPipelineHandle& other) const;

private:
    bool m_initialized = false;
    uint32_t m_value;
};

class Renderer;
class Shader;
class DescriptorSetHandle;
class DescriptorSet;
class ShaderDescriptorSet;

class GraphicsPipeline {
public:
    GraphicsPipeline(
        Renderer& renderer,
        const Shader& vertex_shader,
        const Shader& fragment_shader,
        const VertexLayout& vertex_layout);

    GraphicsPipeline(const GraphicsPipeline&) = delete;

    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    GraphicsPipeline& operator=(GraphicsPipeline&& other);

    [[nodiscard]] bool operator==(const GraphicsPipeline& other) const;

    [[nodiscard]] bool operator<(const GraphicsPipeline& other) const;

    GraphicsPipeline(GraphicsPipeline&& other);

    ~GraphicsPipeline();

    [[nodiscard]] GraphicsPipelineHandle handle() const;

    [[nodiscard]] bool is_valid() const;

    DescriptorSet create_descriptor_set(const ShaderDescriptorSet& descriptor_set);

private:
    bool m_valid = false;
    Renderer* m_renderer;
    GraphicsPipelineHandle m_handle;
};

}

namespace std {
template <>
struct hash<mve::GraphicsPipelineHandle> {
    std::size_t operator()(const mve::GraphicsPipelineHandle& handle) const
    {
        return hash<uint32_t>()(handle.value());
    }
};

template <>
struct hash<mve::GraphicsPipeline> {
    std::size_t operator()(const mve::GraphicsPipeline& graphics_pipeline) const
    {
        return hash<mve::GraphicsPipelineHandle>()(graphics_pipeline.handle());
    }
};
}