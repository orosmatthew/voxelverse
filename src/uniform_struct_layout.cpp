#include "uniform_struct_layout.hpp"

#include <glm/glm.hpp>

namespace mve {
UniformStructLayout::UniformStructLayout(const std::string& name)
    : m_name(name)
    , m_size_bytes(0)
{
}

void UniformStructLayout::push_back(const std::string& variable_name, UniformType type)
{
    Variable variable {};
    variable.type = type;

    if (!variable.is_array && !variable.is_struct) {
        size_t base = base_alignment_of(m_size_bytes, variable.type);
        variable.location = UniformLocation(base);
        m_size_bytes += (base - m_size_bytes);
        m_size_bytes += size_of(variable.type);
    }

    // TODO: calculate array and struct locations

    m_variables.insert({ variable_name, std::move(variable) });
}

UniformLocation UniformStructLayout::location_of(const std::string& name) const
{
    return m_variables.at(name).location;
}

size_t UniformStructLayout::size_bytes() const
{
    return m_size_bytes;
}

std::string UniformStructLayout::name() const
{
    return m_name;
}

size_t UniformStructLayout::base_alignment_of(size_t offset, UniformType type)
{
    switch (type) {
    case UniformType::e_float:
        return glm::ceil(offset / static_cast<float>(sizeof(float))) * sizeof(float);
    case UniformType::e_vec2:
        return glm::ceil(offset / static_cast<float>(sizeof(glm::vec2))) * sizeof(glm::vec2);
    case UniformType::e_vec3:
        // NOTE: this is correct, base alignment of vec3 is same as vec4
        return glm::ceil(offset / static_cast<float>(sizeof(glm::vec4))) * sizeof(glm::vec4);
    case UniformType::e_vec4:
        return glm::ceil(offset / static_cast<float>(sizeof(glm::vec4))) * sizeof(glm::vec4);
    case UniformType::e_mat2:
        return glm::ceil(offset / static_cast<float>(sizeof(glm::mat2::col_type))) * sizeof(glm::mat2::col_type);
    case UniformType::e_mat3:
        return glm::ceil(offset / static_cast<float>(sizeof(glm::mat3::col_type))) * sizeof(glm::mat3::col_type);
    case UniformType::e_mat4:
        return glm::ceil(offset / static_cast<float>(sizeof(glm::mat4::col_type))) * sizeof(glm::mat4::col_type);
    default:
        throw std::runtime_error("[UniformStructLayout] Unknown type");
    }
}

size_t UniformStructLayout::size_of(UniformType type)
{
    switch (type) {
    case UniformType::e_float:
        return sizeof(float);
    case UniformType::e_vec2:
        return sizeof(glm::vec2);
    case UniformType::e_vec3:
        return sizeof(glm::vec3);
    case UniformType::e_vec4:
        return sizeof(glm::vec4);
    case UniformType::e_mat2:
        return sizeof(glm::mat2);
    case UniformType::e_mat3:
        return sizeof(glm::mat3);
    case UniformType::e_mat4:
        return sizeof(glm::mat4);
    }
}
}