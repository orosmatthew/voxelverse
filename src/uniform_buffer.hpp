#pragma once

#include <cstdint>
#include <functional>

#include <glm/matrix.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace mve {

class Renderer;
class ShaderDescriptorBinding;
class UniformLocation;

class UniformBuffer {
public:
    UniformBuffer(Renderer& renderer, const ShaderDescriptorBinding& descriptor_binding);

    UniformBuffer(Renderer& renderer, uint64_t handle);

    UniformBuffer(const UniformBuffer&) = delete;

    UniformBuffer(UniformBuffer&& other);

    ~UniformBuffer();

    UniformBuffer& operator=(const UniformBuffer&) = delete;

    UniformBuffer& operator=(UniformBuffer&& other);

    [[nodiscard]] bool operator==(const UniformBuffer& other) const;

    [[nodiscard]] bool operator<(const UniformBuffer& other) const;

    [[nodiscard]] uint64_t handle() const;

    [[nodiscard]] bool is_valid() const;

    void invalidate();

    void update(UniformLocation location, float value, bool persist = true);

    void update(UniformLocation location, glm::vec2 value, bool persist = true);

    void update(UniformLocation location, glm::vec3 value, bool persist = true);

    void update(UniformLocation location, glm::vec4 value, bool persist = true);

    void update(UniformLocation location, glm::mat2 value, bool persist = true);

    void update(UniformLocation location, glm::mat3 value, bool persist = true);

    void update(UniformLocation location, glm::mat4 value, bool persist = true);

private:
    bool m_valid = false;
    Renderer* m_renderer;
    uint64_t m_handle;
};

}

namespace std {
template <>
struct hash<mve::UniformBuffer> {
    std::size_t operator()(const mve::UniformBuffer& uniform_buffer) const
    {
        return hash<uint64_t>()(uniform_buffer.handle());
    }
};
}