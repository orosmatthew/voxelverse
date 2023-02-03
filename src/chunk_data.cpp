#include "chunk_data.hpp"

ChunkData::ChunkData()
{
}
void ChunkData::set_block(mve::Vector3i pos, uint8_t type)
{
    m_block_data[pos.x][pos.y][pos.z] = type;
}
uint8_t ChunkData::get_block(mve::Vector3i pos) const
{
    return m_block_data[pos.x][pos.y][pos.z];
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
                set_block({ x, y, z }, generator.get_block({ x, y, z }));
            }
        }
    }
}
mve::Vector3i direction_vector(Direction dir)
{
    switch (dir) {
    case Direction::front:
        return { 0, -1, 0 };
    case Direction::back:
        return { 0, 1, 0 };
    case Direction::left:
        return { -1, 0, 0 };
    case Direction::right:
        return { 1, 0, 0 };
    case Direction::top:
        return { 0, 0, 1 };
    case Direction::bottom:
        return { 0, 0, -1 };
    default:
        return { 0, 0, 0 };
    }
}
Direction opposite_direction(Direction dir)
{
    switch (dir) {
    case Direction::front:
        return Direction::back;
    case Direction::back:
        return Direction::front;
    case Direction::left:
        return Direction::right;
    case Direction::right:
        return Direction::left;
    case Direction::top:
        return Direction::bottom;
    case Direction::bottom:
        return Direction::top;
    default:
        return Direction::front;
    }
}
