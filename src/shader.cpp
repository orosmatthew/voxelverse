#include "shader.hpp"

#include <fstream>
#include <vector>

#include "logger.hpp"

namespace mve {

Shader::Shader(const std::filesystem::path& file_path, ShaderType shader_type)
{
    LOG->debug("Loading shader: " + file_path.string());

    std::ifstream file(file_path, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + file_path.string());
    }

    std::vector<uint32_t> contents;
    uint32_t data;
    while (file.read(reinterpret_cast<char*>(&data), sizeof(data))) {
        contents.push_back(data);
    }

    file.close();

    m_spv_code = contents;
};

std::vector<uint32_t> Shader::spv_code() const noexcept
{
    return m_spv_code;
}

}