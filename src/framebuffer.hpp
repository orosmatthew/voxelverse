#pragma once

#include <functional>

namespace mve {

class Renderer;

class Framebuffer {
public:
    Framebuffer(Renderer& renderer);

    Framebuffer(Renderer& renderer, size_t handle);

    Framebuffer(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&& other);

    ~Framebuffer();

    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer& operator=(Framebuffer&& other);

    [[nodiscard]] bool operator==(const Framebuffer& other) const;

    [[nodiscard]] bool operator<(const Framebuffer&& other) const;

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
struct hash<mve::Framebuffer> {
    std::size_t operator()(const mve::Framebuffer& framebuffer) const
    {
        return hash<size_t>()(framebuffer.handle());
    }
};
}