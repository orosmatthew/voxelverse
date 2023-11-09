#pragma once

#include <vector>

#include "math/math.hpp"

namespace mve {

enum class VertexAttributeType {
    scalar,
    vec2,
    vec3,
    vec4,
};

using VertexLayout = std::vector<VertexAttributeType>;

[[nodiscard]] int get_vertex_layout_bytes(const VertexLayout& vertex_layout);

class VertexData {
public:
    explicit VertexData(VertexLayout layout);

    void push_back(float value);

    void push_back(Vector2 value);

    void push_back(Vector3 value);

    void push_back(Vector4 value);

    [[nodiscard]] VertexAttributeType next_type() const noexcept;

    [[nodiscard]] const float* data_ptr() const noexcept;

    [[nodiscard]] int data_count() const noexcept;

    [[nodiscard]] int vertex_count() const noexcept;

    [[nodiscard]] bool is_complete() const noexcept;

    [[nodiscard]] VertexLayout layout() const noexcept;

private:
    VertexLayout m_layout;
    std::vector<float> m_data;
    int m_data_count = 0;
};
}
