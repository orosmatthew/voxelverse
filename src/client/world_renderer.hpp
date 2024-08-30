#pragma once

#include <unordered_map>

#include <BS_thread_pool.hpp>

#include "common.hpp"

#include <nnm/nnm.hpp>

#include "chunk_mesh.hpp"
#include "frustum.hpp"
#include "player.hpp"
#include "wire_box_mesh.hpp"

class WorldRenderer {
public:
    explicit WorldRenderer(mve::Renderer& renderer);

    void push_mesh_update(nnm::Vector3i chunk_pos);

    void process_mesh_updates(const WorldData& world_data);

    bool contains_data(nnm::Vector3i position) const;

    void remove_data(nnm::Vector3i position);

    void set_view(const nnm::Matrix4f& view);

    void resize();

    void set_selection_position(nnm::Vector3f position);

    void hide_selection()
    {
        m_selection_box.is_shown = false;
    }

    void show_selection()
    {
        m_selection_box.is_shown = true;
    }

    void draw(const Player& camera);

    static mve::VertexLayout vertex_layout()
    {
        return {
            mve::VertexAttributeType::vec3, // Position
            mve::VertexAttributeType::vec3, // Color
            mve::VertexAttributeType::vec2, // UV
            mve::VertexAttributeType::scalar // Lighting
        };
    }

    uint64_t create_debug_box(const BoundingBox& box, float width, nnm::Vector3f color);

    void hide_debug_box(uint64_t id);

    void show_debug_box(uint64_t id);

    void delete_debug_box(uint64_t id);

    void delete_all_debug_boxes();

private:
    struct SelectionBox {
        bool is_shown;
        WireBoxMesh mesh;
    };

    struct DebugBox {
        bool is_shown;
        WireBoxMesh mesh;
    };

    // void rebuild_mesh_lookup();

    mve::Renderer* m_renderer;
    BS::thread_pool m_thread_pool;
    mve::Shader m_vertex_shader;
    mve::Shader m_fragment_shader;
    mve::GraphicsPipeline m_graphics_pipeline;
    std::shared_ptr<mve::Texture> m_block_texture;
    mve::UniformBuffer m_global_ubo;
    mve::UniformBuffer m_chunk_ubo;
    mve::DescriptorSet m_global_descriptor_set;
    mve::DescriptorSet m_chunk_descriptor_set;
    mve::UniformLocation m_view_location;
    mve::UniformLocation m_proj_location;
    std::vector<std::optional<ChunkBufferData>> m_temp_chunk_buffer_data {};
    std::unordered_map<nnm::Vector3i, size_t> m_chunk_mesh_lookup {};
    std::vector<std::optional<ChunkBuffers>> m_chunk_buffers {};
    Frustum m_frustum;
    SelectionBox m_selection_box;
    std::unordered_map<uint64_t, DebugBox> m_debug_boxes {};
    std::vector<nnm::Vector3i> m_chunk_mesh_update_list {};
};