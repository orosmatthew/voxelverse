#pragma once

#include <unordered_map>

#include "chunk_mesh.hpp"
#include "math/vector3i.hpp"

class WorldRenderer {
public:
    WorldRenderer(mve::Renderer& renderer);

    void add_data(const ChunkData& chunk_data, const WorldData& world_data);

    void set_view(const mve::Matrix4& view);

    void resize();

    void draw();

    static mve::VertexLayout chunk_vertex_layout();

private:
    mve::Renderer* m_renderer;
    mve::Shader m_vertex_shader;
    mve::Shader m_fragment_shader;
    mve::GraphicsPipeline m_graphics_pipeline;
    std::shared_ptr<mve::Texture> m_block_texture;
    mve::UniformBuffer m_global_ubo;
    mve::DescriptorSet m_global_descriptor_set;
    mve::UniformLocation m_view_location;
    mve::UniformLocation m_proj_location;
    std::unordered_map<mve::Vector3i, ChunkMesh> m_chunk_meshes {};
};