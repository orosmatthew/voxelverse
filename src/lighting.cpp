#include "lighting.hpp"

#include "chunk_column.hpp"

void apply_sunlight(ChunkColumn& chunk)
{
    for_2d({ 0, 0 }, { 16, 16 }, [&](const mve::Vector2i offset) {
        const mve::Vector2i world_col = block_local_to_world_col(chunk.pos(), offset);
        bool covered = false;
        for (int i = 9 * 16; i >= -10 * 16; --i) {
            const mve::Vector3i world_pos { world_col.x, world_col.y, i };
            if (!covered) {
                chunk.set_lighting(world_pos, 15);
                if (!is_transparent(chunk.get_block(world_pos))) {
                    covered = true;
                }
            }
            else {
                chunk.set_lighting(world_pos, 11);
            }
        }
    });
}