#include "chunk_data.hpp"
#include "common.hpp"

ChunkData::ChunkData()
    : m_pos(mve::Vector3i(0.0f))
{
    reset_lighting();
}

ChunkData::ChunkData(mve::Vector3i chunk_pos)
    : m_pos(chunk_pos)
{
    reset_lighting();
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

    //    if (is_emissive(type)
    //        && std::find(m_emissive_blocks.begin(), m_emissive_blocks.end(), pos) == m_emissive_blocks.end()) {
    //        m_emissive_blocks.push_back(pos);
    //    }
    //    else if (type == 0) {
    //        auto it = std::find(m_emissive_blocks.begin(), m_emissive_blocks.end(), pos);
    //        if (it != m_emissive_blocks.end()) {
    //            m_emissive_blocks.erase(it);
    //        }
    //    }

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
