#include "vertex_data.hpp"

#include <stdexcept>

namespace mve {

int get_vertex_layout_bytes(const VertexLayout& vertex_layout)
{
    int byte_count = 0;
    for (VertexAttributeType type : vertex_layout) {
        switch (type) {
        case VertexAttributeType::e_float:
            byte_count += sizeof(float);
            break;
        case VertexAttributeType::e_vec2:
            byte_count += sizeof(glm::vec2);
            break;
        case VertexAttributeType::e_vec3:
            byte_count += sizeof(glm::vec3);
            break;
        case VertexAttributeType::e_vec4:
            byte_count += sizeof(glm::vec4);
            break;
        }
    }
    return byte_count;
}

VertexData::VertexData(VertexLayout layout)
    : m_layout(std::move(layout))
{
    if (m_layout.empty()) {
        throw std::runtime_error("[VertexData] Empty vertex layout");
    }
}

void VertexData::push_back(float value)
{
    if (next_type() != VertexAttributeType::e_float) {
        throw std::runtime_error("[VertexData] Invalid type: float");
    }

    m_data.push_back(value);

    m_data_count++;
}

void VertexData::push_back(glm::vec2 value)
{
    if (next_type() != VertexAttributeType::e_vec2) {
        throw std::runtime_error("[VertexData] Invalid type: vec2");
    }

    m_data.push_back(value[0]);
    m_data.push_back(value[1]);

    m_data_count++;
}

void VertexData::push_back(glm::vec3 value)
{
    if (next_type() != VertexAttributeType::e_vec3) {
        throw std::runtime_error("[VertexData] Invalid type: vec3");
    }

    m_data.push_back(value[0]);
    m_data.push_back(value[1]);
    m_data.push_back(value[2]);

    m_data_count++;
}

void VertexData::push_back(glm::vec4 value)
{
    if (next_type() != VertexAttributeType::e_vec4) {
        throw std::runtime_error("[VertexData] Invalid type: vec4");
    }

    m_data.push_back(value[0]);
    m_data.push_back(value[1]);
    m_data.push_back(value[2]);
    m_data.push_back(value[3]);

    m_data_count++;
}

VertexAttributeType VertexData::next_type() const noexcept
{
    return m_layout[m_data_count % m_layout.size()];
}

const float* VertexData::data_ptr() const noexcept
{
    return m_data.data();
}

int VertexData::data_count() const noexcept
{
    return m_data_count;
}

bool VertexData::is_complete() const noexcept
{
    return (m_data_count % m_layout.size()) == 0;
}

VertexLayout VertexData::layout() const noexcept
{
    return m_layout;
}

int VertexData::vertex_count() const noexcept
{
    return m_data_count / m_layout.size();
}
}