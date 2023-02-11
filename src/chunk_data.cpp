#include "chunk_data.hpp"

ChunkData::ChunkData(mve::Vector3i chunk_pos)
    : m_pos(chunk_pos)
{
}
void ChunkData::set_block(mve::Vector3i pos, uint8_t type)
{
    if (m_block_data[pos.x][pos.y][pos.z] == 0 && type == 1) {
        m_block_count++;
    }
    else if (m_block_data[pos.x][pos.y][pos.z] == 1 && type == 0) {
        m_block_count--;
    }
    m_block_data[pos.x][pos.y][pos.z] = type;
}
bool ChunkData::in_bounds(mve::Vector3i pos) const
{
    return pos.x >= 0 && pos.x < 16 && pos.y >= 0 && pos.y < 16 && pos.z >= 0 && pos.z < 16;
}
void ChunkData::generate(const WorldGenerator& generator)
{
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                set_block({ x, y, z }, generator.get_block(mve::Vector3i(x, y, z) + m_pos * 16, true));
            }
        }
    }
}
mve::Vector3i ChunkData::position() const
{
    return m_pos;
}
