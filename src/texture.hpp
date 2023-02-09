#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>

namespace mve {

class Renderer;

class Texture {

    friend Renderer;

public:
    Texture(Renderer& renderer, const std::filesystem::path& path);

    Texture(Renderer& renderer, uint64_t handle);

    Texture(const Texture&) = delete;

    Texture(Texture&& other);

    ~Texture();

    Texture& operator=(const Texture&) = delete;

    Texture& operator=(Texture&& other);

    [[nodiscard]] bool operator==(const Texture& other) const;

    [[nodiscard]] bool operator<(const Texture& other) const;

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
struct hash<mve::Texture> {
    std::size_t operator()(const mve::Texture& texture) const
    {
        return hash<uint64_t>()(texture.handle());
    }
};

}