#pragma once

#include <array>

#include "common.hpp"

#include <nnm/nnm.hpp>

#include <mve/renderer.hpp>

class WorldData;
struct ChunkFaceData {
    std::array<nnm::Vector3f, 4> vertices;
    std::array<nnm::Vector3f, 4> colors;
    std::array<nnm::Vector2f, 4> uvs;
    std::array<uint32_t, 6> indices {};
};

struct ChunkMeshData {
    std::vector<nnm::Vector3f> vertices;
    std::vector<nnm::Vector3f> colors;
    std::vector<nnm::Vector2f> uvs;
    std::vector<uint32_t> indices;
};

struct ChunkBufferData {
    nnm::Vector3i chunk_pos;
    mve::VertexData vertex_data;
    std::vector<uint32_t> index_data;
};

class ChunkBuffers {
public:
    ChunkBuffers(mve::Renderer& renderer, const ChunkBufferData& buffer_data)
        : m_chunk_pos(buffer_data.chunk_pos)
        , m_vertex_buffer(renderer.create_vertex_buffer(buffer_data.vertex_data))
        , m_index_buffer(renderer.create_index_buffer(buffer_data.index_data))
    {
    }

    [[nodiscard]] nnm::Vector3i chunk_pos() const
    {
        return m_chunk_pos;
    }

    void draw(mve::Renderer& renderer) const
    {
        renderer.bind_vertex_buffer(m_vertex_buffer);
        renderer.draw_index_buffer(m_index_buffer);
    }

private:
    nnm::Vector3i m_chunk_pos;
    mve::VertexBuffer m_vertex_buffer;
    mve::IndexBuffer m_index_buffer;
};

std::optional<ChunkBufferData> create_chunk_buffer_data(nnm::Vector3i chunk_pos, const WorldData& world_data);