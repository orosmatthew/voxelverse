#pragma once

#include <filesystem>
#include <vector>

namespace mve {

enum class ShaderType {
    vertex,
    fragment,
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
     * @throws std::runtime_error - When file path is invalid
     */
    Shader(const std::filesystem::path& file_path, ShaderType shader_type);

    /**
     * @brief Obtain SPIR-V code from shader
     * @return - returns byte (uint32_t) array of SPIR-V code
     */
    [[nodiscard]] std::vector<uint32_t> spv_code() const noexcept;

private:
    std::vector<uint32_t> m_spv_code;
};

}