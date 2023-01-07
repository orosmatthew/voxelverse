#pragma once

#include <cstdint>
#include <vector>

namespace mve {

class IndexBufferHandle {
public:
    IndexBufferHandle(uint32_t value);

    [[nodiscard]] uint32_t value() const;

    bool operator==(const IndexBufferHandle& other) const;

    bool operator<(const IndexBufferHandle& other) const;

private:
    uint32_t m_value;
};

class Renderer;

class IndexBuffer {
public:
    IndexBuffer(Renderer& renderer, const std::vector<uint32_t>& indices);

    IndexBuffer(const IndexBuffer&) = delete;

    IndexBuffer& operator=(const IndexBuffer&) = delete;

    IndexBuffer& operator=(IndexBuffer&& other);

    [[nodiscard]] bool operator==(const IndexBuffer& other) const;

    [[nodiscard]] bool operator<(const IndexBuffer& other) const;

    IndexBuffer(IndexBuffer&& other);

    ~IndexBuffer();

    [[nodiscard]] IndexBufferHandle handle() const;

    [[nodiscard]] bool is_valid() const;

private:
    bool m_valid = false;
    Renderer* m_renderer;
    IndexBufferHandle m_handle;
};

}

namespace std {
template <>
struct hash<mve::IndexBufferHandle> {
    std::size_t operator()(const mve::IndexBufferHandle& handle) const
    {
        return hash<uint32_t>()(handle.value());
    }
};

template <>
struct hash<mve::IndexBuffer> {
    std::size_t operator()(const mve::IndexBuffer& index_buffer) const
    {
        return hash<mve::IndexBufferHandle>()(index_buffer.handle());
    }
};
}