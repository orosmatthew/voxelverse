#include "lighting.hpp"

#include "chunk_column.hpp"
#include "world_data.hpp"

void apply_sunlight(ChunkColumn& chunk)
{
    for_2d({ 0, 0 }, { 16, 16 }, [&](const nnm::Vector2i offset) {
        const nnm::Vector2i world_col = block_local_to_world_col(chunk.pos(), offset);
        bool covered = false;
        for (int i = 9 * 16; i >= -10 * 16; --i) {
            const nnm::Vector3i world_pos { world_col.x, world_col.y, i };
            if (!covered) {
                if (!is_transparent(chunk.get_block(world_pos))) {
                    covered = true;
                }
                else {
                    chunk.set_lighting(world_pos, 15);
                }
            }
            else {
                chunk.set_lighting(world_pos, 0);
            }
        }
    });
}

void propagate_light(WorldData& world_data, const nnm::Vector3i chunk_pos)
{
    static std::vector<std::pair<nnm::Vector3i, uint8_t>> queue;
    queue.clear();

    const std::array<nnm::Vector3i, 6> adjacent
        = { { { 0, 0, 1 }, { 0, 0, -1 }, { 0, 1, 0 }, { 0, -1, 0 }, { 1, 0, 0 }, { -1, 0, 0 } } };

    std::array<nnm::Vector3i, 26> surr_pos {};
    {
        int i = 0;
        for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](const nnm::Vector3i pos) {
            if (pos != nnm::Vector3i::zero()) {
                surr_pos[i] = pos;
                ++i;
            }
        });
    }

    ChunkData& current_chunk_data = world_data.chunk_data_at(chunk_pos);
    std::array<std::optional<ChunkData*>, 26> surr_chunks {};
    for (int i = 0; i < surr_pos.size(); ++i) {
        if (world_data.contains_chunk(chunk_pos + surr_pos[i])) {
            surr_chunks[i] = &world_data.chunk_data_at(chunk_pos + surr_pos[i]);
        }
    }

    auto fast_lighting_at = [&](const nnm::Vector3i pos) -> std::optional<uint8_t> {
        const nnm::Vector3i offset = chunk_pos_from_block_pos(pos) - chunk_pos;
        const nnm::Vector3i local_pos = block_world_to_local(pos);
        if (offset == nnm::Vector3i(0, 0, 0)) {
            return current_chunk_data.lighting_at(local_pos);
        }
        for (int i = 0; i < surr_chunks.size(); ++i) {
            if (offset == surr_pos[i]) {
                return surr_chunks[i].has_value()
                    ? surr_chunks[i].value()->lighting_at(local_pos)
                    : std::optional<uint8_t> {};
            }
        }
        VV_DEB_ASSERT(false, "Unreachable");
        return {};
    };

    auto fast_block_at = [&](const nnm::Vector3i pos) -> std::optional<uint8_t> {
        const nnm::Vector3i offset = chunk_pos_from_block_pos(pos) - chunk_pos;
        const nnm::Vector3i local_pos = block_world_to_local(pos);
        if (offset == nnm::Vector3i(0, 0, 0)) {
            return current_chunk_data.get_block(local_pos);
        }
        for (int i = 0; i < surr_chunks.size(); ++i) {
            if (offset == surr_pos[i]) {
                return surr_chunks[i].has_value()
                    ? surr_chunks[i].value()->get_block(local_pos)
                    : std::optional<uint8_t> {};
            }
        }
        VV_DEB_ASSERT(false, "Unreachable");
        return {};
    };

    auto fast_set_lighting = [&](const nnm::Vector3i pos, const uint8_t val) {
        const nnm::Vector3i offset = chunk_pos_from_block_pos(pos) - chunk_pos;
        const nnm::Vector3i local_pos = block_world_to_local(pos);
        if (offset == nnm::Vector3i(0, 0, 0)) {
            return current_chunk_data.set_lighting(local_pos, val);
        }
        for (int i = 0; i < surr_chunks.size(); ++i) {
            if (offset == surr_pos[i]) {
                surr_chunks[i].value()->set_lighting(local_pos, val);
                return;
            }
        }
        VV_DEB_ASSERT(false, "Unreachable");
    };

    for_3d({ 0, 0, 0 }, { 16, 16, 16 }, [&](const nnm::Vector3i pos) {
        const nnm::Vector3i world_pos = block_local_to_world(chunk_pos, pos);
        if (const std::optional<uint8_t> block = fast_block_at(world_pos);
            fast_lighting_at(world_pos) >= 15 || (block.has_value() && block.value() == 9)) {
            queue.emplace_back(world_pos, 15);
        }
    });

    while (!queue.empty()) {
        const auto [pos, prev_val] = queue.back();
        queue.pop_back();
        for (const nnm::Vector3i offset : adjacent) {
            const nnm::Vector3i adj_pos = pos + offset;
            const std::optional<uint8_t> current_lighting = fast_lighting_at(adj_pos);
            // ReSharper disable once CppTooWideScopeInitStatement
            const std::optional<uint8_t> block_type = fast_block_at(adj_pos);
            if (block_type.has_value() && current_lighting.has_value() && current_lighting < prev_val - 1
                && is_transparent(block_type.value())) {
                fast_set_lighting(adj_pos, prev_val - 1);
                if (prev_val - 1 > 1) {
                    queue.emplace_back(adj_pos, prev_val - 1);
                }
            }
        }
    }
}

void refresh_lighting(WorldData& world_data, const nnm::Vector3i chunk_pos)
{
    // TODO: Make lighting queue and need to do whole column

    for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](const nnm::Vector3i offset) {
        if (world_data.contains_chunk(chunk_pos + offset)) {
            world_data.chunk_data_at(chunk_pos + offset).reset_lighting(0);
        }
    });

    for_2d({ -1, -1 }, { 2, 2 }, [&](const nnm::Vector2i offset) {
        if (world_data.contains_column({ chunk_pos.x + offset.x, chunk_pos.y + offset.y })) {
            apply_sunlight(world_data.chunk_column_data_at({ chunk_pos.x + offset.x, chunk_pos.y + offset.y }));
        }
    });

    for_3d({ -1, -1, -1 }, { 2, 2, 2 }, [&](const nnm::Vector3i offset) {
        if (world_data.contains_chunk(chunk_pos + offset)) {
            propagate_light(world_data, chunk_pos + offset);
        }
    });
}