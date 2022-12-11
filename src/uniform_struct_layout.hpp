#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <strong_type/strong_type.hpp>

namespace mve {

/**
 * @brief Primitive types for a uniform struct
 */
enum class UniformType {
    e_float,
    e_vec2,
    e_vec3,
    e_vec4,
    e_mat2,
    e_mat3,
    e_mat4,
};

/**
 * @brief The location of the uniform in a uniform struct (like a pointer)
 */
using UniformLocation = strong::type<size_t, struct _uniform_location, strong::regular, strong::hashable>;

/**
 * @brief Class to represent a uniform struct layout
 */
class UniformStructLayout {
public:
    /**
     * @brief Construct UniformStructLayout
     * @param name - name of the struct
     */
    UniformStructLayout(const std::string& name) noexcept;

    /**
     * @brief Add a type to the struct
     * @param name - name of the uniform variable
     * @throws std::runtime_error - When a duplicate name is added
     * @param type - type of the uniform variable
     */
    void push_back(const std::string& name, UniformType type);

    /**
     * @brief Get the location of a uniform struct variable
     * @param name - name of the variable
     * @throws std::runtime_error - When name is not valid
     * @return - Returns the uniform location of the struct variable
     */
    [[nodiscard]] UniformLocation location_of(const std::string& name) const;

    /**
     * @brief Get the type of a uniform struct variable
     * @param name - name of the variable
     * @throws std::runtime_error - When name is not valid
     * @return - Returns the uniform type
     */
    [[nodiscard]] UniformType type_of(const std::string& name) const;

    /**
     * @brief Get the size of the struct in bytes
     * @return - Returns the size of the struct in bytes
     */
    [[nodiscard]] size_t size_bytes() const noexcept;

    /**
     * @brief Get the name of the struct
     * @return - Returns the name of the struct
     */
    [[nodiscard]] std::string name() const noexcept;

private:
    struct Variable {
        UniformType type;
        UniformLocation location;

        bool is_array = false;
        size_t array_size = 0;

        bool is_struct = false;
        std::unique_ptr<UniformStructLayout> struct_data = nullptr;
    };

    const std::string m_name;
    std::map<std::string, Variable> m_variables;
    size_t m_size_bytes;

    static size_t base_alignment_of(size_t offset, UniformType type);

    inline static size_t size_of(UniformType type);
};

}