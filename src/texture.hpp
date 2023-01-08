#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>

namespace mve {

class TextureHandle {
public:
    TextureHandle();

    TextureHandle(uint32_t value);

    void set(uint32_t value);

    [[nodiscard]] uint32_t value() const;

    [[nodiscard]] bool operator==(const TextureHandle& other) const;

    [[nodiscard]] bool operator<(const TextureHandle& other) const;

private:
    bool m_initialized = false;
    uint32_t m_value;
};

class Renderer;

class Texture {
public:
    Texture(Renderer& renderer, const std::filesystem::path& path);

    Texture(const Texture&) = delete;

    Texture(Texture&& other);

    ~Texture();

    Texture& operator=(const Texture&) = delete;

    Texture& operator=(Texture&& other);

    [[nodiscard]] bool operator==(const Texture& other) const;

    [[nodiscard]] bool operator<(const Texture& other) const;

    [[nodiscard]] TextureHandle handle() const;

    [[nodiscard]] bool is_valid() const;

private:
    bool m_valid = false;
    Renderer* m_renderer;
    TextureHandle m_handle;
};

}

namespace std {
template <>
struct hash<mve::TextureHandle> {
    std::size_t operator()(const mve::TextureHandle& handle) const
    {
        return hash<uint32_t>()(handle.value());
    }
};

}