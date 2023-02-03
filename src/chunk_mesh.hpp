#pragma once

#include <memory>

#include "chunk_data.hpp"
#include "descriptor_set.hpp"
#include "graphics_pipeline.hpp"
#include "index_buffer.hpp"
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
        std::vector<mve::Vector2> uvs;
        std::vector<uint32_t> indices;
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

    static FaceUVs uvs_from_atlas(int tex_width, int tex_height, int atlas_width, int atlas_height, int x, int y);

    static void combine_mesh_data(MeshData& data, const MeshData& other);

    static MeshData create_face_mesh(mve::Vector3 offset, BlockFace face);

    static MeshBuffers create_buffers_from_chunk_data(mve::Renderer& renderer, const ChunkData& chunk_data);

    mve::DescriptorSet m_descriptor_set;
    mve::UniformBuffer m_uniform_buffer;
    MeshBuffers m_mesh_buffers;
    std::shared_ptr<mve::Texture> m_texture;
    mve::UniformLocation m_model_location;
};
