#include "save_file.hpp"

#include <filesystem>

#include "mve/common.hpp"

SaveFile::SaveFile(size_t max_file_size, const std::string& name)
{
    if (!std::filesystem::exists("save")) {
        std::filesystem::create_directory("save");
    }
    leveldb::Options db_options;
    db_options.create_if_missing = true;
    db_options.compression = leveldb::kNoCompression;
    db_options.max_file_size = max_file_size;
    leveldb::Status db_status = leveldb::DB::Open(db_options, "save/" + name, &m_db);
    MVE_ASSERT(db_status.ok(), "[SaveFile] Leveldb open not ok for " + name);
}
SaveFile::~SaveFile()
{
    delete m_db;
}
std::optional<std::string> SaveFile::at(const std::string& key)
{
    std::string data;
    leveldb::Status db_status = m_db->Get(leveldb::ReadOptions(), key, &data);
    if (db_status.IsNotFound()) {
        return {};
    }
    MVE_ASSERT(db_status.ok(), "[SaveFile] Failed to get key: " + key)
    return data;
}

void SaveFile::insert(const std::string& key, const std::string& value)
{
    if (m_writing_batch) {
        m_batch.Put(key, value);
    }
    else {
        leveldb::Status db_status = m_db->Put(leveldb::WriteOptions(), key, value);
        MVE_ASSERT(db_status.ok(), "[SaveFile] Failed to write key: " + key);
    }
}

void SaveFile::begin_batch()
{
    m_writing_batch = true;
    clear_batch();
}

void SaveFile::submit_batch()
{
    leveldb::Status db_status = m_db->Write(leveldb::WriteOptions(), &m_batch);
    clear_batch();
    m_writing_batch = false;
}

void SaveFile::clear_batch()
{
    m_batch.Clear();
    m_writing_batch = false;
}
