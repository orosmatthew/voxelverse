#pragma once

#include <optional>
#include <stdint.h>
#include <string>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

// TODO: Include compression

class SaveFile {
public:
    SaveFile(size_t max_file_size, const std::string& name);

    ~SaveFile();

    std::optional<std::string> at(const std::string& key);

    void insert(const std::string& key, const std::string& value);

    void begin_batch();

    void submit_batch();

    void clear_batch();

private:
    bool m_writing_batch = false;
    leveldb::WriteBatch m_batch {};
    leveldb::DB* m_db;
};