#pragma once

#include <array>

#include "mve/math/math.hpp"
#include "mve/renderer.hpp"

class WorldData;
struct ChunkFaceData {
    std::array<mve::Vector3, 4> vertices;
    std::array<mve::Vector3, 4> colors;
    std::array<mve::Vector2, 4> uvs;
    std::array<uint32_t, 6> indices {};
};

struct ChunkMeshData {
    std::vector<mve::Vector3> vertices;
    std::vector<mve::Vector3> colors;
    std::vector<mve::Vector2> uvs;
    std::vector<uint32_t> indices;
};

struct ChunkBufferData {
    mve::Vector3i chunk_pos;
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

    [[nodiscard]] mve::Vector3i chunk_pos() const
    {
        return m_chunk_pos;
    }

    void draw(mve::Renderer& renderer) const
    {
        renderer.bind_vertex_buffer(m_vertex_buffer);
        renderer.draw_index_buffer(m_index_buffer);
    }

private:
    mve::Vector3i m_chunk_pos;
    mve::VertexBuffer m_vertex_buffer;
    mve::IndexBuffer m_index_buffer;
};

std::optional<ChunkBufferData> create_chunk_buffer_data(mve::Vector3i chunk_pos, const WorldData& world_data);