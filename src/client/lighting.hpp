#pragma once

#include <mve/math/math.hpp>

class WorldData;
class ChunkColumn;

void apply_sunlight(ChunkColumn& chunk);

void propagate_light(WorldData& world_data, mve::Vector3i chunk_pos);

void refresh_lighting(WorldData& world_data, mve::Vector3i chunk_pos);
