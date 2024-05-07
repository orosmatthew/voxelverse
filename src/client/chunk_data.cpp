#include "chunk_data.hpp"

ChunkData::ChunkData()
{
    // reset_lighting(15);
}

ChunkData::ChunkData(const mve::Vector3i chunk_pos)
    : m_pos(chunk_pos)
{
    // reset_lighting(15);
}
void ChunkData::set_block(const mve::Vector3i pos, const uint8_t type)
{
    VV_DEB_ASSERT(is_block_pos_local(pos), "[ChunkData] Invalid local block position");
    if (m_block_data[index(pos)] == 0 && type != 0) {
        m_block_count++;
    }
    else if (m_block_data[index(pos)] != 0 && type == 0) {
        m_block_count--;
    }
    m_block_data[index(pos)] = type;
}
