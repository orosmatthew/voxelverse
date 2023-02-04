#pragma once

#include <array>
#include <memory>

#include "chunk_data.hpp"
#include "descriptor_set.hpp"
#include "graphics_pipeline.hpp"
#include "index_buffer.hpp"
#include "math/matrix4.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "uniform_buffer.hpp"
#include "vertex_buffer.hpp"

class ChunkMesh {
public:
    ChunkMesh(
        const ChunkData& data,
        mve::Renderer& renderer,
        mve::GraphicsPipeline& pipeline,
        mve::Shader& vertex_shader,
        mve::Shader& fragment_shader,
        std::shared_ptr<mve::Texture> texture);

    void draw(mve::Renderer& renderer, mve::DescriptorSet& global_descriptor_set);

private:
    struct MeshData {
        std::vector<mve::Vector3> vertices;
        std::vector<mve::Vector3> colors;
        std::vector<mve::Vector2> uvs;
        std::vector<uint32_t> indices;
    };
    struct FaceData {
        std::array<mve::Vector3, 4> vertices;
        std::array<mve::Vector3, 4> colors;
        std::array<mve::Vector2, 4> uvs;
        std::array<uint32_t, 6> indices;
    };
    struct MeshBuffers {
        mve::VertexBuffer vertex_buffer;
        mve::IndexBuffer index_buffer;
    };
    struct FaceUVs {
        mve::Vector2 top_left;
        mve::Vector2 top_right;
        mve::Vector2 bottom_right;
        mve::Vector2 bottom_left;
    };

    static FaceUVs uvs_from_atlas(mve::Vector2i texture_size, mve::Vector2i atlas_size, mve::Vector2i pos);

    static void combine_mesh_data(MeshData& data, const MeshData& other);

    static void add_face_to_mesh(MeshData& data, const FaceData& face);

    static FaceData create_face_mesh(mve::Vector3 offset, Direction face, const std::array<uint8_t, 4>& lighting);

    static std::array<uint8_t, 4> calc_face_lighting(const ChunkData& data, mve::Vector3i block_pos, Direction dir);

    static std::optional<MeshBuffers> create_buffers_from_chunk_data(
        mve::Renderer& renderer, const ChunkData& chunk_data);

    mve::Matrix4 m_transform;
    mve::DescriptorSet m_descriptor_set;
    mve::UniformBuffer m_uniform_buffer;
    std::optional<MeshBuffers> m_mesh_buffers;
    std::shared_ptr<mve::Texture> m_texture;
    mve::UniformLocation m_model_location;
};
