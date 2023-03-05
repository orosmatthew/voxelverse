#include "world_data.hpp"

#include <filesystem>

#include <cereal/archives/portable_binary.hpp>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <lz4.h>

#include "mve/common.hpp"

WorldData::WorldData()
{
    MVE_ASSERT(std::filesystem::exists("save"), "[WorldData] save dir does not exist")
    leveldb::Options db_options;
    db_options.create_if_missing = true;
    db_options.compression = leveldb::kNoCompression;
    db_options.max_file_size = 16 * 1024 * 1024; // 16 MB
    leveldb::Status db_status = leveldb::DB::Open(db_options, "save/world_data", &m_save_db);
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
bool WorldData::try_load_chunk_from_save(mve::Vector3i chunk_pos)
{
    std::stringstream key_stream;
    {
        cereal::PortableBinaryOutputArchive archive_out(key_stream);
        archive_out(chunk_pos);
    }
    std::string key = key_stream.str();
    std::string compressed_data;
    leveldb::Status status = m_save_db->Get(leveldb::ReadOptions(), key, &compressed_data);
    if (status.IsNotFound()) {
        return false;
    }
    MVE_ASSERT(status.ok(), "[WorldData] Failed to load chunk from save")
    std::vector<char> decompressed_data;
    decompressed_data.resize(5000); // TODO: Do this programmatically
    int result_size = LZ4_decompress_safe(
        compressed_data.data(), decompressed_data.data(), compressed_data.size(), decompressed_data.size());
    MVE_ASSERT(result_size >= 0, "[WorldData] Failed to decompress chunk data from save")
    decompressed_data.resize(result_size);
    std::stringstream data(std::string(decompressed_data.begin(), decompressed_data.end()));
    ChunkData chunk_data({ 0, 0, 0 }); // TODO: Have default constructor
    {
        cereal::PortableBinaryInputArchive archive_in(data);
        archive_in(chunk_data);
    }
    // TODO: remove duplicate callback from create_chunk()
    chunk_data.set_modified_callback([this](mve::Vector3i chunk_pos, const ChunkData& chunk_data) {
        queue_save_chunk(chunk_pos);
    });
    m_chunks.insert({ chunk_pos, std::move(chunk_data) });
    return true;
}
