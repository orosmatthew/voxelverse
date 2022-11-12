#include "vertex_data.hpp"

#include <stdexcept>

namespace mve {

    VertexData::VertexData(VertexLayout layout)
        : m_layout(std::move(layout))
    {
    }

    void VertexData::add_data(float value)
    {
        assert(get_next_type() == VertexAttributeType::e_float);

        m_data.push_back(value);

        m_type_count++;
    }

    void VertexData::add_data(glm::vec2 value)
    {
        assert(get_next_type() == VertexAttributeType::e_vec2);

        m_data.push_back(value[0]);
        m_data.push_back(value[1]);

        m_type_count++;
    }

    void VertexData::add_data(glm::vec3 value)
    {
        assert(get_next_type() == VertexAttributeType::e_vec3);

        m_data.push_back(value[0]);
        m_data.push_back(value[1]);
        m_data.push_back(value[2]);

        m_type_count++;
    }

    void VertexData::add_data(glm::vec4 value)
    {
        assert(get_next_type() == VertexAttributeType::e_vec4);

        m_data.push_back(value[0]);
        m_data.push_back(value[1]);
        m_data.push_back(value[2]);
        m_data.push_back(value[3]);

        m_type_count++;
    }

    VertexAttributeType VertexData::get_next_type() const
    {
        return m_layout[m_type_count % m_layout.size()];
    }

    const float *VertexData::get_data_ptr() const
    {
        return m_data.data();
    }
}