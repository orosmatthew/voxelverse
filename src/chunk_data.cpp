#include "chunk_data.hpp"

ChunkData::ChunkData()
{
}
void ChunkData::set_block(mve::Vector3i pos, uint8_t type)
{
    m_block_data[pos.z][pos.y][pos.x] = type;
}
uint8_t ChunkData::get_block(mve::Vector3i pos) const
{
    return m_block_data[pos.z][pos.y][pos.x];
}
