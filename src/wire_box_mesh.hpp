#pragma once

#include "common.hpp"
#include "mve/math/math.hpp"
#include "mve/renderer.hpp"

class WireBoxMesh {
public:
    WireBoxMesh(
        mve::Renderer& renderer,
        mve::GraphicsPipeline& pipeline,
        const mve::ShaderDescriptorSet& set,
        const mve::ShaderDescriptorBinding& uniform_buffer_binding,
        const BoundingBox& box,
        float width,
        mve::Vector3 color);

    void set_position(mve::Vector3 position);

    void draw(const mve::DescriptorSet& global_set) const;

private:
    struct MeshBuffers {
        mve::VertexBuffer vertex_buffer;
        mve::IndexBuffer index_buffer;
    };
    struct MeshData {
        std::vector<mve::Vector3> vertices;
        std::vector<uint32_t> indices;
    };

    static MeshData create_rect_mesh(float length, float width, const mve::Matrix4& matrix);

    static MeshData create_rect_mesh(mve::Vector3 from, mve::Vector3 to, float width);

    static void combine_mesh_data(MeshData& data, const MeshData& other);

    mve::Renderer* m_renderer;
    mve::DescriptorSet m_descriptor_set;
    mve::UniformBuffer m_uniform_buffer;
    mve::UniformLocation m_model_location;
    std::optional<MeshBuffers> m_mesh_buffers;
};