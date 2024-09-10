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
    UniformLocation()
        : m_value()
    {
    }

    explicit UniformLocation(const uint32_t value)
        : m_initialized(true)
        , m_value(value)

    {
    }

    void set(const uint32_t value)
    {
        m_initialized = true;
        m_value = value;
    }

    [[nodiscard]] uint32_t value() const
    {
        return m_value;
    }

    [[nodiscard]] bool operator==(const UniformLocation& other) const
    {
        return m_value == other.m_value;
    }

    [[nodiscard]] bool operator<(const UniformLocation& other) const
    {
        return m_value < other.m_value;
    }

private:
    bool m_initialized = false;
    uint32_t m_value;
};

class ShaderBindingBlock {
public:
    ShaderBindingBlock(
        std::string name,
        const uint32_t size,
        const uint32_t offset,
        std::unordered_map<std::string, ShaderBindingBlock> members)
        : m_name(std::move(name))
        , m_size(size)
        , m_offset(offset)
        , m_members(std::move(members))
    {
    }

    [[nodiscard]] std::string name() const
    {
        return m_name;
    }

    [[nodiscard]] uint32_t size() const
    {
        return m_size;
    }

    [[nodiscard]] uint32_t offset() const
    {
        return m_offset;
    }

    [[nodiscard]] UniformLocation location() const
    {
        return UniformLocation(offset());
    }

    [[nodiscard]] const ShaderBindingBlock& member(const std::string& name) const
    {
        return m_members.at(name);
    }

    [[nodiscard]] const std::unordered_map<std::string, ShaderBindingBlock>& members() const
    {
        return m_members;
    }

private:
    std::string m_name;
    uint32_t m_size;
    uint32_t m_offset;
    std::unordered_map<std::string, ShaderBindingBlock> m_members;
};

class ShaderDescriptorBinding {
public:
    ShaderDescriptorBinding(
        std::string name, uint32_t binding, ShaderDescriptorType type, std::optional<ShaderBindingBlock> block)
        : m_name(std::move(name))
        , m_binding(binding)
        , m_type(type)
        , m_block(std::move(block))
    {
    }

    [[nodiscard]] std::string name() const
    {
        return m_name;
    }

    [[nodiscard]] uint32_t binding() const
    {
        return m_binding;
    }

    [[nodiscard]] const ShaderDescriptorType& type() const
    {
        return m_type;
    }

    [[nodiscard]] const ShaderBindingBlock& block() const
    {
        return m_block.value();
    }

    [[nodiscard]] const ShaderBindingBlock& member(const std::string& name) const
    {
        return m_block.value().member(name);
    }

    [[nodiscard]] const std::unordered_map<std::string, ShaderBindingBlock>& members() const
    {
        return m_block.value().members();
    }

private:
    std::string m_name;
    uint32_t m_binding;
    ShaderDescriptorType m_type;
    std::optional<ShaderBindingBlock> m_block;
};

class ShaderDescriptorSet {
public:
    ShaderDescriptorSet(const uint32_t set, std::unordered_map<uint32_t, ShaderDescriptorBinding> bindings)
        : m_set(set)
        , m_bindings(std::move(bindings))
    {
    }

    [[nodiscard]] uint32_t set() const
    {
        return m_set;
    }

    [[nodiscard]] uint32_t binding_count() const
    {
        return static_cast<uint32_t>(m_bindings.size());
    }

    [[nodiscard]] const ShaderDescriptorBinding& binding(const uint32_t binding) const
    {
        return m_bindings.at(binding);
    }

    [[nodiscard]] const std::unordered_map<uint32_t, ShaderDescriptorBinding>& bindings() const
    {
        return m_bindings;
    }

private:
    const uint32_t m_set;
    const std::unordered_map<uint32_t, ShaderDescriptorBinding> m_bindings;
};

class Shader {
public:
    explicit Shader(const std::filesystem::path& file_path);

    [[nodiscard]] std::vector<uint32_t> spv_code() const noexcept;

    [[nodiscard]] const std::unordered_map<uint32_t, ShaderDescriptorSet>& descriptor_sets() const;

    [[nodiscard]] const ShaderDescriptorSet& descriptor_set(uint32_t set) const;

    [[nodiscard]] bool has_descriptor_set(uint32_t set) const;

private:
    struct ShaderReflectionData {
        std::unordered_map<uint32_t, ShaderDescriptorSet> sets {};
    };

    void create_reflection_data();

    std::vector<uint32_t> m_spv_code;
    ShaderReflectionData m_reflection_data;
};

}

template <>
struct std::hash<mve::UniformLocation> {
    std::size_t operator()(const mve::UniformLocation& location) const noexcept
    {
        return hash<uint32_t>()(location.value());
    }
};