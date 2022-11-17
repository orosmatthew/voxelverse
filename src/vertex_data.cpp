#include "vertex_data.hpp"

#include <stdexcept>

namespace mve {

    int get_vertex_layout_bytes(const VertexLayout &vertex_layout)
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
    }

    void VertexData::add_data(float value)
    {
        assert(get_next_type() == VertexAttributeType::e_float);

        m_data.push_back(value);

        m_data_count++;
    }

    void VertexData::add_data(glm::vec2 value)
    {
        assert(get_next_type() == VertexAttributeType::e_vec2);

        m_data.push_back(value[0]);
        m_data.push_back(value[1]);

        m_data_count++;
    }

    void VertexData::add_data(glm::vec3 value)
    {
        assert(get_next_type() == VertexAttributeType::e_vec3);

        m_data.push_back(value[0]);
        m_data.push_back(value[1]);
        m_data.push_back(value[2]);

        m_data_count++;
    }

    void VertexData::add_data(glm::vec4 value)
    {
        assert(get_next_type() == VertexAttributeType::e_vec4);

        m_data.push_back(value[0]);
        m_data.push_back(value[1]);
        m_data.push_back(value[2]);
        m_data.push_back(value[3]);

        m_data_count++;
    }

    VertexAttributeType VertexData::get_next_type() const
    {
        return m_layout[m_data_count % m_layout.size()];
    }

    const float *VertexData::get_data_ptr() const
    {
        return m_data.data();
    }

    int VertexData::get_count() const
    {
        return m_data_count;
    }

    bool VertexData::is_complete() const
    {
        return (m_data_count % m_layout.size()) == 0;
    }

    VertexLayout VertexData::get_layout() const
    {
        return m_layout;
    }
}