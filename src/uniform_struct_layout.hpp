#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <strong_type/strong_type.hpp>

namespace mve {

    enum class UniformType {
        e_float,
        e_vec2,
        e_vec3,
        e_vec4,
        e_mat2,
        e_mat3,
        e_mat4,
    };

    using UniformLocation = strong::type<size_t, struct _uniform_location, strong::regular, strong::hashable>;

    class UniformStructLayout {
    public:
        UniformStructLayout(const std::string& name);

        void push_back(const std::string& name, UniformType type);

        [[nodiscard]] UniformLocation location_of(const std::string& name) const;

        [[nodiscard]] size_t size_bytes() const;

        [[nodiscard]] std::string name() const;

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
        std::unordered_map<std::string, Variable> m_variables;
        size_t m_size_bytes;

        static size_t base_alignment_of(size_t offset, UniformType type);
    };

}