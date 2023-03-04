#include "world_data.hpp"

#include <cereal/archives/portable_binary.hpp>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <lz4.h>

#include "mve/common.hpp"

WorldData::WorldData()
{
    leveldb::Options db_options;
    db_options.create_if_missing = true;
    db_options.compression = leveldb::kNoCompression;
    leveldb::Status db_status = leveldb::DB::Open(db_options, "save", &m_save_db);
    MVE_ASSERT(db_status.ok(), "[App] Leveldb open not ok")
}

void WorldData::queue_save_chunk(mve::Vector3i pos)
{
    m_save_queue.insert(pos);
    if (m_save_queue.size() > 50) {
        process_save_queue();
    }
}

WorldData::~WorldData()
{
    process_save_queue();
    delete m_save_db;
}
void WorldData::process_save_queue()
{
    leveldb::WriteBatch batch;
    for (mve::Vector3i pos : m_save_queue) {
        const ChunkData& data = m_chunks.at(pos);
        std::stringstream serial_stream;
        {
            cereal::PortableBinaryOutputArchive archive_out(serial_stream);
            archive_out(data);
        }
        std::string serial_data = serial_stream.str();
        std::vector<char> compressed_data(LZ4_compressBound(serial_data.size()));
        int compressed_size = LZ4_compress_default(
            serial_data.data(), compressed_data.data(), serial_data.size(), compressed_data.size());
        MVE_ASSERT(compressed_size > 0, "[App] LZ4 compression error")
        compressed_data.resize(compressed_size);
        std::stringstream key_stream;
        {
            cereal::PortableBinaryOutputArchive archive_out(key_stream);
            archive_out(pos);
        }
        std::string key_data = key_stream.str();
        batch.Put(key_data, leveldb::Slice(compressed_data.data(), compressed_data.size()));
    }
    leveldb::Status db_status = m_save_db->Write(leveldb::WriteOptions(), &batch);
    MVE_ASSERT(db_status.ok(), "[App] Leveldb write not ok")
    m_save_queue.clear();
}
