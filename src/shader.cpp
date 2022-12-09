#include "shader.hpp"

#include <fstream>

#include "logger.hpp"

namespace mve {

    Shader::Shader(const std::filesystem::path& file_path, ShaderType shader_type)
    {
        LOG->debug("Loading shader: " + file_path.string());

        auto file = std::ifstream(file_path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + file_path.string());
        }

        size_t file_size = (size_t)file.tellg();
        auto buffer = std::vector<char>(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();

        m_spv_code = buffer;
    };

    std::vector<char> Shader::get_spv_code() const
    {
        return m_spv_code;
    }

}