#include "uniform_struct_layout.hpp"

#include <glm/common.hpp>
#include <glm/mat2x2.hpp>

namespace mve {
UniformStructLayout::UniformStructLayout(const std::string& name) noexcept
    : m_name(name)
    , m_size_bytes(0)
{
}

void UniformStructLayout::push_back(const std::string& variable_name, UniformType type)
{
    if (m_variables.contains(variable_name)) {
        throw std::runtime_error("[UniformStructLayout] Variable name already exists.");
    }

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
    try {
        return m_variables.at(name).location;
    }
    catch (std::out_of_range& e) {
        throw std::runtime_error("[UniformLayout] Invalid variable name: " + name);
    }
}

size_t UniformStructLayout::size_bytes() const noexcept
{
    return m_size_bytes;
}

std::string UniformStructLayout::name() const noexcept
{
    return m_name;
}

size_t UniformStructLayout::base_alignment_of(size_t offset, UniformType type)
{
    switch (type) {
    case UniformType::scalar:
        return glm::ceil(offset / static_cast<float>(sizeof(float))) * sizeof(float);
    case UniformType::vec2:
        return glm::ceil(offset / static_cast<float>(sizeof(glm::vec2))) * sizeof(glm::vec2);
    case UniformType::vec3:
        // NOTE: this is correct, base alignment of vec3 is same as vec4
        return glm::ceil(offset / static_cast<float>(sizeof(glm::vec4))) * sizeof(glm::vec4);
    case UniformType::vec4:
        return glm::ceil(offset / static_cast<float>(sizeof(glm::vec4))) * sizeof(glm::vec4);
    case UniformType::mat2:
        return glm::ceil(offset / static_cast<float>(sizeof(glm::mat2::col_type))) * sizeof(glm::mat2::col_type);
    case UniformType::mat3:
        return glm::ceil(offset / static_cast<float>(sizeof(glm::mat3::col_type))) * sizeof(glm::mat3::col_type);
    case UniformType::mat4:
        return glm::ceil(offset / static_cast<float>(sizeof(glm::mat4::col_type))) * sizeof(glm::mat4::col_type);
    default:
        throw std::runtime_error("[UniformStructLayout] Unknown type.");
    }
}

size_t UniformStructLayout::size_of(UniformType type)
{
    switch (type) {
    case UniformType::scalar:
        return sizeof(float);
    case UniformType::vec2:
        return sizeof(glm::vec2);
    case UniformType::vec3:
        return sizeof(glm::vec3);
    case UniformType::vec4:
        return sizeof(glm::vec4);
    case UniformType::mat2:
        return sizeof(glm::mat2);
    case UniformType::mat3:
        return sizeof(glm::mat3);
    case UniformType::mat4:
        return sizeof(glm::mat4);
    default:
        return 0;
    }
}

UniformType UniformStructLayout::type_of(const std::string& name) const
{
    try {
        return m_variables.at(name).type;
    }
    catch (std::out_of_range& e) {
        throw std::runtime_error("[UniformStructLayout] Invalid variable name: " + name);
    }
}

}