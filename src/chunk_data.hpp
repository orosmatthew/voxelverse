#pragma once

#include <stdint.h>

class ChunkData {
public:
    ChunkData();

private:
    static const int sc_chunk_size = 16;
    uint8_t m_block_data[sc_chunk_size][sc_chunk_size][sc_chunk_size] = { 0 };
};