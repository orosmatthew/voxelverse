#pragma once

#include <cstdint>
#include <functional>

namespace mve {

class VertexBufferHandle {
public:
    VertexBufferHandle();

    VertexBufferHandle(uint32_t value);

    void set(uint32_t value);

    [[nodiscard]] uint32_t value() const;

    [[nodiscard]] bool operator==(const VertexBufferHandle& other) const;

    [[nodiscard]] bool operator<(const VertexBufferHandle& other) const;

private:
    bool m_initialized = false;
    uint32_t m_value;
};

class Renderer;
class VertexData;

class VertexBuffer {
public:
    VertexBuffer(Renderer& renderer, const VertexData& data);

    VertexBuffer(Renderer& renderer, VertexBufferHandle handle);

    VertexBuffer(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& other);

    ~VertexBuffer();

    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer& operator=(VertexBuffer&& other);

    [[nodiscard]] bool operator==(const VertexBuffer& other) const;

    [[nodiscard]] bool operator<(const VertexBuffer& other) const;

    [[nodiscard]] VertexBufferHandle handle() const;

    [[nodiscard]] bool is_valid() const;

private:
    bool m_valid = false;
    Renderer* m_renderer;
    VertexBufferHandle m_handle;
};

}

namespace std {
template <>
struct hash<mve::VertexBufferHandle> {
    std::size_t operator()(const mve::VertexBufferHandle& handle) const
    {
        return hash<uint32_t>()(handle.value());
    }
};

template <>
struct hash<mve::VertexBuffer> {
    std::size_t operator()(const mve::VertexBuffer& vertex_buffer) const
    {
        return hash<mve::VertexBufferHandle>()(vertex_buffer.handle());
    }
};
}