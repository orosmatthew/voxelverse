#pragma once

#include <filesystem>
#include <vector>

namespace mve {

    enum class ShaderType {
        e_vertex,
        e_fragment,
    };

    /**
     * @brief Shader for use in renderer
     */
    class Shader {
    public:
        /**
         * @brief Construct Shader from file
         * @param file_path - file path of shader file
         * @param shader_type - type of shader
         */
        Shader(const std::filesystem::path& file_path, ShaderType shader_type);

        /**
         * @brief Obtain SPIR-V code from shader
         * @return - returns byte (char) array of SPIR-V code
         */
        [[nodiscard]] std::vector<char> get_spv_code() const;

    private:
        std::vector<char> m_spv_code;
    };

}