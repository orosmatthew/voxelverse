#pragma once

#include <cstdint>
#include <vector>

namespace mve {

class Renderer;

class IndexBuffer {
public:
    IndexBuffer(Renderer& renderer, const std::vector<uint32_t>& indices);

    IndexBuffer(Renderer& renderer, uint64_t handle);

    IndexBuffer(const IndexBuffer&) = delete;

    IndexBuffer(IndexBuffer&& other);

    ~IndexBuffer();

    IndexBuffer& operator=(const IndexBuffer&) = delete;

    IndexBuffer& operator=(IndexBuffer&& other);

    [[nodiscard]] bool operator==(const IndexBuffer& other) const;

    [[nodiscard]] bool operator<(const IndexBuffer& other) const;

    [[nodiscard]] uint64_t handle() const;

    [[nodiscard]] bool is_valid() const;

    void invalidate();

private:
    bool m_valid = false;
    Renderer* m_renderer;
    uint64_t m_handle;
};
}

namespace std {
template <>
struct hash<mve::IndexBuffer> {
    std::size_t operator()(const mve::IndexBuffer& index_buffer) const
    {
        return hash<uint64_t>()(index_buffer.handle());
    }
};
}