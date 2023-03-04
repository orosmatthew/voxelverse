#include "chunk_data.hpp"

ChunkData::ChunkData(mve::Vector3i chunk_pos)
    : m_pos(chunk_pos)
{
}
void ChunkData::set_block(mve::Vector3i pos, uint8_t type)
{
    if (m_block_data[index(pos)] == 0 && type != 0) {
        m_block_count++;
    }
    else if (m_block_data[index(pos)] != 0 && type == 0) {
        m_block_count--;
    }
    m_block_data[index(pos)] = type;
    if (m_modified_callback.has_value()) {
        std::invoke(*m_modified_callback, m_pos, *this);
    }
}
bool ChunkData::in_bounds(mve::Vector3i pos) const
{
    return pos.x >= 0 && pos.x < 16 && pos.y >= 0 && pos.y < 16 && pos.z >= 0 && pos.z < 16;
}
mve::Vector3i ChunkData::position() const
{
    return m_pos;
}
