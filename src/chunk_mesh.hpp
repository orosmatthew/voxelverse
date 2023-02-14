#pragma once

#include <array>
#include <memory>

#include "descriptor_set.hpp"
#include "graphics_pipeline.hpp"
#include "index_buffer.hpp"
#include "math/math.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "uniform_buffer.hpp"
#include "vertex_buffer.hpp"
#include "world_data.hpp"

class ChunkMesh {
public:
    ChunkMesh(mve::Vector3i chunk_pos, const WorldData& data, mve::Renderer& renderer);

    void draw(mve::Renderer& renderer) const;

    inline mve::Vector3i chunk_position() const
    {
        return m_chunk_pos;
    }

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

    static void combine_mesh_data(MeshData& data, const MeshData& other);

    static void add_face_to_mesh(MeshData& data, const FaceData& face);

    static FaceData create_face_mesh(uint8_t block_type, mve::Vector3 offset, Direction face, const std::array<uint8_t, 4>& lighting);

    static void calc_block_faces(
        uint8_t block_type,
        MeshData& mesh,
        const WorldData& world_data,
        const ChunkData& chunk_data,
        mve::Vector3i chunk_pos,
        mve::Vector3i local_pos,
        bool iterate_empty,
        const std::array<bool, 6>& directions = { true, true, true, true, true, true });

    static std::array<uint8_t, 4> calc_face_lighting(
        const WorldData& data,
        const ChunkData& chunk_data,
        mve::Vector3i chunk_pos,
        mve::Vector3i local_block_pos,
        Direction dir);

    static std::optional<MeshBuffers> create_buffers(
        mve::Vector3i chunk_pos, mve::Renderer& renderer, const WorldData& world_data);

    mve::Vector3i m_chunk_pos;
    std::optional<MeshBuffers> m_mesh_buffers;
};
