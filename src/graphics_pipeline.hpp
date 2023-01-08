#pragma once

#include <cstdint>
#include <functional>

#include "vertex_data.hpp"

namespace mve {

class Renderer;
class Shader;
class DescriptorSet;
class ShaderDescriptorSet;

class GraphicsPipeline {
public:
    GraphicsPipeline(
        Renderer& renderer,
        const Shader& vertex_shader,
        const Shader& fragment_shader,
        const VertexLayout& vertex_layout);

    GraphicsPipeline(Renderer& renderer, uint64_t handle);

    GraphicsPipeline(const GraphicsPipeline&) = delete;

    GraphicsPipeline(GraphicsPipeline&& other);

    ~GraphicsPipeline();

    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    GraphicsPipeline& operator=(GraphicsPipeline&& other);

    [[nodiscard]] bool operator==(const GraphicsPipeline& other) const;

    [[nodiscard]] bool operator<(const GraphicsPipeline& other) const;

    [[nodiscard]] uint64_t handle() const;

    [[nodiscard]] bool is_valid() const;

    void invalidate();

    DescriptorSet create_descriptor_set(const ShaderDescriptorSet& descriptor_set);

private:
    bool m_valid = false;
    Renderer* m_renderer;
    uint64_t m_handle;
};

}

namespace std {
template <>
struct hash<mve::GraphicsPipeline> {
    std::size_t operator()(const mve::GraphicsPipeline& graphics_pipeline) const
    {
        return hash<uint64_t>()(graphics_pipeline.handle());
    }
};
}