#pragma once

#include <optional>
#include <sstream>
#include <string>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
// ReSharper disable once CppUnusedIncludeDirective
#include <cereal/types/string.hpp>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

class SaveFile {
public:
    SaveFile(size_t max_file_size, const std::string& name);

    ~SaveFile();

    template <typename KeyType, typename ValueType, typename... Args>
    std::optional<ValueType> at(const KeyType& key, Args&&... args)
    {
        std::stringstream key_stream;
        {
            cereal::PortableBinaryOutputArchive archive_out(key_stream);
            archive_out(key);
        }
        std::optional<std::string> value_str = at(key_stream.str());
        if (!value_str.has_value()) {
            return {};
        }
        ValueType value(std::forward<Args>(args)...);
        {
            std::stringstream value_stream(*value_str);
            cereal::PortableBinaryInputArchive archive_in(value_stream);
            archive_in(value);
        }
        return value;
    }

    template <typename KeyType>
    std::optional<std::string> at(const KeyType& key)
    {
        std::stringstream key_stream;
        {
            cereal::PortableBinaryOutputArchive archive_out(key_stream);
            archive_out(key);
        }
        return at(key_stream.str());
    }

    std::optional<std::string> at(const std::string& key);

    template <typename KeyType, typename ValueType>
    void insert(const KeyType& key, const ValueType& value)
    {
        std::stringstream key_stream;
        {
            cereal::PortableBinaryOutputArchive archive_out(key_stream);
            archive_out(key);
        }
        std::stringstream value_stream;
        {
            cereal::PortableBinaryOutputArchive archive_out(value_stream);
            archive_out(value);
        }
        insert(key_stream.str(), value_stream.str());
    }

    void insert(const std::string& key, const std::string& value);

    void begin_batch();

    void submit_batch();

    void clear_batch();

private:
    struct ValueData {
        size_t decompressed_size;
        std::string data;

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(decompressed_size, data);
        }
    };

    bool m_writing_batch = false;
    leveldb::WriteBatch m_batch {};
    leveldb::DB* m_db {};
};