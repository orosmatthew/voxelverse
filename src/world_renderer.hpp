#pragma once

#include <unordered_map>

#include "camera.hpp"
#include "chunk_mesh.hpp"
#include "common.hpp"
#include "frustum.hpp"
#include "math/math.hpp"

class WorldRenderer {
public:
    WorldRenderer(mve::Renderer& renderer);

    void add_data(const ChunkData& chunk_data, const WorldData& world_data);

    void set_view(const mve::Matrix4& view);

    void resize();

    void draw(const Camera& camera);

    inline static mve::VertexLayout chunk_vertex_layout()
    {
        return {
            mve::VertexAttributeType::vec3, // Position
            mve::VertexAttributeType::vec3, // Color
            mve::VertexAttributeType::vec2 // UV
        };
    }

private:
    void rebuild_mesh_lookup();

    mve::Renderer* m_renderer;
    mve::Shader m_vertex_shader;
    mve::Shader m_fragment_shader;
    mve::GraphicsPipeline m_graphics_pipeline;
    std::shared_ptr<mve::Texture> m_block_texture;
    mve::UniformBuffer m_global_ubo;
    mve::DescriptorSet m_global_descriptor_set;
    mve::UniformLocation m_view_location;
    mve::UniformLocation m_proj_location;
    std::unordered_map<mve::Vector3i, size_t> m_chunk_mesh_lookup {};
    std::vector<ChunkMesh> m_chunk_meshes;
    Frustum m_frustum;
};