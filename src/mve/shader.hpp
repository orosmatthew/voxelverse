#pragma once

#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>

namespace mve {

enum class ShaderType {
    vertex,
    fragment,
};

enum class ShaderDescriptorType {
    uniform_buffer,
    combined_image_sampler,
};

class UniformLocation {
public:
    UniformLocation();

    explicit UniformLocation(uint32_t value);

    void set(uint32_t value);

    [[nodiscard]] uint32_t value() const;

    [[nodiscard]] bool operator==(const UniformLocation& other) const;

    [[nodiscard]] bool operator<(const UniformLocation& other) const;

private:
    bool m_initialized = false;
    uint32_t m_value;
};

class ShaderBindingBlock {
public:
    ShaderBindingBlock(
        const std::string& name,
        uint32_t size,
        uint32_t offset,
        std::unordered_map<std::string, ShaderBindingBlock> members);

    [[nodiscard]] std::string name() const;

    [[nodiscard]] uint32_t size() const;

    [[nodiscard]] uint32_t offset() const;

    [[nodiscard]] UniformLocation location() const;

    [[nodiscard]] const ShaderBindingBlock& member(const std::string& name) const;

    [[nodiscard]] const std::unordered_map<std::string, ShaderBindingBlock>& members() const;

private:
    std::string m_name;
    uint32_t m_size;
    uint32_t m_offset;
    std::unordered_map<std::string, ShaderBindingBlock> m_members;
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
    std::string m_name;
    uint32_t m_binding;
    ShaderDescriptorType m_type;
    std::optional<ShaderBindingBlock> m_block;
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

// TODO: Move shader reflection data to its own class
/**
 * @brief Shader for use in renderer
 */
class Shader {
public:
    /**
     * @brief Construct Shader from file
     * @param file_path - file path of shader file
     * @throws std::runtime_error - When file path is invalid
     */
    explicit Shader(const std::filesystem::path& file_path);

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

namespace std {
template <>
struct hash<mve::UniformLocation> {
    std::size_t operator()(const mve::UniformLocation& location) const
    {
        return hash<uint32_t>()(location.value());
    }
};
}