#pragma once

#include <cstdint>
#include <functional>

namespace mve {

class Renderer;
class VertexData;

class VertexBuffer {

    friend Renderer;

public:
    VertexBuffer(Renderer& renderer, const VertexData& vertex_data);

    VertexBuffer(Renderer& renderer, size_t handle);

    VertexBuffer(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& other);

    ~VertexBuffer();

    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer& operator=(VertexBuffer&& other);

    [[nodiscard]] bool operator==(const VertexBuffer& other) const;

    [[nodiscard]] bool operator<(const VertexBuffer& other) const;

    [[nodiscard]] size_t handle() const;

    [[nodiscard]] bool is_valid() const;

    void invalidate();

private:
    bool m_valid = false;
    Renderer* m_renderer;
    size_t m_handle;
};

}

namespace std {

template <>
struct hash<mve::VertexBuffer> {
    std::size_t operator()(const mve::VertexBuffer& vertex_buffer) const
    {
        return hash<uint64_t>()(vertex_buffer.handle());
    }
};
}