#include "save_file.hpp"

#include <filesystem>

#include <cereal/archives/portable_binary.hpp>
#include <lz4.h>

#include "mve/common.hpp"

SaveFile::SaveFile(const size_t max_file_size, const std::string& name)
{
    if (!std::filesystem::exists("save")) {
        std::filesystem::create_directory("save");
    }
    leveldb::Options db_options;
    db_options.create_if_missing = true;
    db_options.compression = leveldb::kNoCompression;
    db_options.max_file_size = max_file_size;
    const leveldb::Status db_status = leveldb::DB::Open(db_options, "save/" + name, &m_db);
    MVE_ASSERT(db_status.ok(), "[SaveFile] Leveldb open not ok for " + name)
}
SaveFile::~SaveFile()
{
    delete m_db;
}
// ReSharper disable once CppMemberFunctionMayBeConst
std::optional<std::string> SaveFile::at(const std::string& key)
{
    std::string data;
    leveldb::Status db_status = m_db->Get(leveldb::ReadOptions(), key, &data);
    if (db_status.IsNotFound()) {
        return {};
    }
    MVE_ASSERT(db_status.ok(), "[SaveFile] Failed to get key: " + key)
    ValueData value_data;
    {
        std::stringstream data_stream(data);
        cereal::PortableBinaryInputArchive archive_in(data_stream);
        archive_in(value_data);
    }

    std::vector<char> decompressed_data;
    decompressed_data.resize(value_data.decompressed_size);
    int result_size = LZ4_decompress_safe(
        value_data.data.data(),
        decompressed_data.data(),
        static_cast<int>(value_data.data.size()),
        // ReSharper disable once CppRedundantCastExpression
        static_cast<int>(decompressed_data.size()));
    MVE_ASSERT(result_size >= 0, "[SaveFile] Failed to decompress data at key: " + key)
    decompressed_data.resize(result_size);

    return std::string(decompressed_data.begin(), decompressed_data.end());
}

void SaveFile::insert(const std::string& key, const std::string& value)
{

    std::vector<char> compressed_data(LZ4_compressBound(static_cast<int>(value.size())));
    const int compressed_size = LZ4_compress_default(
        value.data(),
        compressed_data.data(),
        static_cast<int>(value.size()),
        // ReSharper disable once CppRedundantCastExpression
        static_cast<int>(compressed_data.size()));
    MVE_ASSERT(compressed_size > 0, "[SaveFile] LZ4 compression error")
    compressed_data.resize(compressed_size);

    ValueData value_data = ValueData { .decompressed_size = value.size(),
                                       .data = std::string(compressed_data.begin(), compressed_data.end()) };

    std::stringstream data_stream;
    {
        cereal::PortableBinaryOutputArchive archive_out(data_stream);
        archive_out(value_data);
    }
    if (m_writing_batch) {
        m_batch.Put(key, data_stream.str());
    }
    else {
        const leveldb::Status db_status = m_db->Put(leveldb::WriteOptions(), key, data_stream.str());
        MVE_ASSERT(db_status.ok(), "[SaveFile] Failed to write key: " + key)
    }
}

void SaveFile::begin_batch()
{
    m_writing_batch = true;
    clear_batch();
}

void SaveFile::submit_batch()
{
    // ReSharper disable once CppDFAUnusedValue
    leveldb::Status db_status = m_db->Write(leveldb::WriteOptions(), &m_batch);
    clear_batch();
    m_writing_batch = false;
}

void SaveFile::clear_batch()
{
    m_batch.Clear();
    m_writing_batch = false;
}
