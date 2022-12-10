#include "uniform_buffer_layout.hpp"

#include <glm/glm.hpp>

#include "logger.hpp"

namespace mve {
    UniformStructLayout::UniformStructLayout(const std::string& name)
        : m_name(name)
        , m_location_offset(0)
        , m_size_bytes(0)
    {
    }

    void UniformStructLayout::push_back(const std::string& variable_name, UniformType type)
    {
        Variable variable {};
        variable.type = type;
        variable.location = calculate_location(variable);

        m_variables.insert({ variable_name, std::move(variable) });
    }

    UniformLocation UniformStructLayout::get_location(const std::string& name) const
    {
        return m_variables.at(name).location;
    }

    UniformLocation UniformStructLayout::calculate_location(const UniformStructLayout::Variable& variable)
    {
        if (!variable.is_array && !variable.is_struct) {
            UniformLocation location;
            size_t base;

            switch (variable.type) {
            case UniformType::e_float:
                base = glm::ceil(m_location_offset / (float)sizeof(float)) * sizeof(float);
                location = UniformLocation(base);
                m_size_bytes += (base - m_size_bytes) + sizeof(float);
                m_location_offset += (base - m_location_offset) + sizeof(float);
                return location;
            case UniformType::e_vec2:
                base = glm::ceil(m_location_offset / (float)sizeof(glm::vec2)) * sizeof(glm::vec2);
                location = UniformLocation(base);
                m_size_bytes += (base - m_size_bytes) + sizeof(glm::vec2);
                m_location_offset += (base - m_location_offset) + sizeof(glm::vec2);
                return location;
            case UniformType::e_vec3:
                // NOTE: this is correct, base alignment of vec3 is same as vec4
                base = glm::ceil(m_location_offset / (float)sizeof(glm::vec4)) * sizeof(glm::vec4);
                location = UniformLocation(base);
                m_size_bytes += (base - m_size_bytes) + sizeof(glm::vec3);
                m_location_offset += (base - m_location_offset) + sizeof(glm::vec4);
                return location;
            case UniformType::e_vec4:
                base = glm::ceil(m_location_offset / (float)sizeof(glm::vec4)) * sizeof(glm::vec4);
                location = UniformLocation(base);
                m_size_bytes += (base - m_size_bytes) + sizeof(glm::vec4);
                m_location_offset += (base - m_location_offset) + sizeof(glm::vec4);
                return location;
            case UniformType::e_mat2:
                base = glm::ceil(m_location_offset / (float)sizeof(glm::mat2::col_type)) * sizeof(glm::mat2::col_type);
                location = UniformLocation(base);
                m_size_bytes += (base - m_size_bytes) + sizeof(glm::mat2);
                m_location_offset += (base - m_location_offset) + sizeof(glm::mat2);
                return location;
            case UniformType::e_mat3:
                base = glm::ceil(m_location_offset / (float)sizeof(glm::mat3::col_type)) * sizeof(glm::mat3::col_type);
                location = UniformLocation(base);
                m_size_bytes += (base - m_size_bytes) + sizeof(glm::mat3);
                m_location_offset += (base - m_location_offset) + sizeof(glm::mat3);
                return location;
            case UniformType::e_mat4:
                base = glm::ceil(m_location_offset / (float)sizeof(glm::mat4::col_type)) * sizeof(glm::mat4::col_type);
                location = UniformLocation(base);
                m_size_bytes += (base - m_size_bytes) + sizeof(glm::mat4);
                m_location_offset += (base - m_location_offset) + sizeof(glm::mat4);
                return location;
            }
        }

        return UniformLocation(0);
    }
    size_t UniformStructLayout::size_bytes() const
    {
        return m_size_bytes;
    }

    std::string UniformStructLayout::name() const
    {
        return m_name;
    }
}