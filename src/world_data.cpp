#include "world_data.hpp"

#include "math/math.hpp"

WorldData::WorldData()
{
}

void WorldData::generate(const WorldGenerator& generator, mve::Vector3i from, mve::Vector3i to)
{
    for (int x = from.x; x < to.x; x++) {
        for (int y = from.y; y < to.y; y++) {
            for (int z = from.z; z < to.z; z++) {
                ChunkData chunk_data({ x, y, z });
                chunk_data.generate(generator);
                m_chunks.insert({ { x, y, z }, std::move(chunk_data) });
            }
        }
    }
}