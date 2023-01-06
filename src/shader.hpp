#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

#include <strong_type/strong_type.hpp>

namespace mve {

enum class ShaderType {
    vertex,
    fragment,
};

enum class ShaderDescriptorType {
    uniform_buffer,
    combined_image_sampler,
};

using UniformLocation = strong::type<uint32_t, struct _uniform_location, strong::regular, strong::hashable>;

class ShaderBindingBlock {
public:
    ShaderBindingBlock(
        std::string name, uint32_t size, uint32_t offset, std::unordered_map<std::string, ShaderBindingBlock> members);

    [[nodiscard]] std::string name() const;

    [[nodiscard]] uint32_t size() const;

    [[nodiscard]] uint32_t offset() const;

    [[nodiscard]] UniformLocation location() const;

    [[nodiscard]] const ShaderBindingBlock& member(const std::string& name) const;

    [[nodiscard]] const std::unordered_map<std::string, ShaderBindingBlock>& members() const;

private:
    const std::string m_name;
    const uint32_t m_size;
    const uint32_t m_offset;
    const std::unordered_map<std::string, ShaderBindingBlock> m_members;
};

class ShaderDescriptorBinding {
public:
    ShaderDescriptorBinding(
        std::string name, uint32_t binding, ShaderDescriptorType type, std::optional<ShaderBindingBlock> block);

    [[nodiscard]] std::string name() const;

    [[nodiscard]] uint32_t binding() const;

    [[nodiscard]] const ShaderDescriptorType& type() const;

    [[nodiscard]] const ShaderBindingBlock& block() const;

    [[nodiscard]] const ShaderBindingBlock& member(const std::string& name) const;

    [[nodiscard]] const std::unordered_map<std::string, ShaderBindingBlock>& members() const;

private:
    const std::string m_name;
    const uint32_t m_binding;
    const ShaderDescriptorType m_type;
    const std::optional<ShaderBindingBlock> m_block;
};

class ShaderDescriptorSet {
public:
    ShaderDescriptorSet(uint32_t set, std::unordered_map<uint32_t, ShaderDescriptorBinding> bindings);

    [[nodiscard]] uint32_t set() const;

    [[nodiscard]] uint32_t binding_count() const;

    [[nodiscard]] const ShaderDescriptorBinding& binding(uint32_t binding) const;

    [[nodiscard]] const std::unordered_map<uint32_t, ShaderDescriptorBinding>& bindings() const;

private:
    const uint32_t m_set;
    const std::unordered_map<uint32_t, ShaderDescriptorBinding> m_bindings;
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

    [[nodiscard]] const std::unordered_map<uint32_t, ShaderDescriptorSet>& descriptor_sets() const;

    [[nodiscard]] const ShaderDescriptorSet& descriptor_set(uint32_t set) const;

    [[nodiscard]] const bool has_descriptor_set(uint32_t set) const;

private:
    struct ShaderReflectionData {
        std::unordered_map<uint32_t, ShaderDescriptorSet> sets {};
    };

    void create_reflection_data();

    std::vector<uint32_t> m_spv_code;
    ShaderReflectionData m_reflection_data;
};

}