#pragma once

#include <span>
#include <vector>

#include "common.hpp"

#include <nnm/nnm.hpp>

namespace mve {

enum class VertexAttributeType {
    scalar,
    vector2,
    vector3,
    vector4,
};

[[nodiscard]] inline int vertex_layout_bytes(const std::span<const VertexAttributeType> vertex_layout)
{
    int byte_count = 0;
    for (const VertexAttributeType type : vertex_layout) {
        switch (type) {
        case VertexAttributeType::scalar:
            byte_count += sizeof(float);
            break;
        case VertexAttributeType::vector2:
            byte_count += sizeof(nnm::Vector2f);
            break;
        case VertexAttributeType::vector3:
            byte_count += sizeof(nnm::Vector3f);
            break;
        case VertexAttributeType::vector4:
            byte_count += sizeof(nnm::Vector4f);
            break;
        }
    }
    return byte_count;
}

class VertexData {
public:
    explicit VertexData(const std::span<const VertexAttributeType> layout)
        : m_layout(layout.begin(), layout.end())
    {
        MVE_ASSERT(!m_layout.empty(), "[VertexData] Empty vertex layout")
    }

    void push(const float value)
    {
        MVE_VAL_ASSERT(next_type() == VertexAttributeType::scalar, "[VertexData] Invalid type: scalar")
        m_data.push_back(value);
        m_attributes++;
    }

    void push(const nnm::Vector2f& value)
    {
        MVE_VAL_ASSERT(next_type() == VertexAttributeType::vector2, "[VertexData] Invalid type: vec2")
        m_data.push_back(value[0]);
        m_data.push_back(value[1]);
        m_attributes++;
    }

    void push(const nnm::Vector3f& value)
    {
        MVE_VAL_ASSERT(next_type() == VertexAttributeType::vector3, "[VertexData] Invalid type: vec3")
        m_data.push_back(value[0]);
        m_data.push_back(value[1]);
        m_data.push_back(value[2]);
        m_attributes++;
    }

    void push(const nnm::Vector4f& value)
    {
        MVE_VAL_ASSERT(next_type() == VertexAttributeType::vector4, "[VertexData] Invalid type: vec4")
        m_data.push_back(value[0]);
        m_data.push_back(value[1]);
        m_data.push_back(value[2]);
        m_data.push_back(value[3]);
        m_attributes++;
    }

    [[nodiscard]] VertexAttributeType next_type() const
    {
        return m_layout[m_attributes % m_layout.size()];
    }

    [[nodiscard]] const float* data() const
    {
        return m_data.data();
    }

    [[nodiscard]] int attributes() const
    {
        return m_attributes;
    }

    [[nodiscard]] int vertex_count() const
    {
        return m_attributes / static_cast<int>(m_layout.size());
    }

    [[nodiscard]] bool complete() const
    {
        return m_attributes % m_layout.size() == 0;
    }

    [[nodiscard]] std::span<const VertexAttributeType> layout() const
    {
        return m_layout;
    }

private:
    std::vector<VertexAttributeType> m_layout;
    std::vector<float> m_data;
    int m_attributes = 0;
};
}
