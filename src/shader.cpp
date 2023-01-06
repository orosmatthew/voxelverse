#include "shader.hpp"

#include <fstream>
#include <optional>
#include <vector>

#include <spirv_reflect.h>

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
    create_reflection_data();
};

std::vector<uint32_t> Shader::spv_code() const noexcept
{
    return m_spv_code;
}

static ShaderDescriptorType convert_descriptor_type(SpvReflectDescriptorType type)
{
    switch (type) {
    case (SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER):
        return ShaderDescriptorType::uniform_buffer;
    case (SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER):
        return ShaderDescriptorType::combined_image_sampler;
    default:
        throw std::runtime_error("[Shader] Failed to convert descriptor type");
    }
}

static std::optional<ShaderBindingBlock> create_binding_block(SpvReflectBlockVariable reflect_block)
{
    std::unordered_map<std::string, ShaderBindingBlock> members;
    for (uint32_t m = 0; m < reflect_block.member_count; m++) {
        std::optional<ShaderBindingBlock> member = create_binding_block(reflect_block.members[m]);
        std::string name = member.value().name();
        members.insert({ name, std::move(member.value()) });
    }

    ShaderBindingBlock block(reflect_block.name, reflect_block.size, reflect_block.offset, std::move(members));
    return block;
}

void Shader::create_reflection_data()
{
    SpvReflectShaderModule shader_module;
    SpvReflectResult shader_module_result
        = spvReflectCreateShaderModule(m_spv_code.size() * 4, m_spv_code.data(), &shader_module);
    if (shader_module_result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("[Shader] Failed to generate reflection data");
    }

    uint32_t var_count = 0;
    SpvReflectResult input_enum_result = spvReflectEnumerateInputVariables(&shader_module, &var_count, nullptr);
    if (input_enum_result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("[Shader] Failed to enumerate input variables");
    }

    std::vector<SpvReflectInterfaceVariable*> input_vars(var_count);
    input_enum_result = spvReflectEnumerateInputVariables(&shader_module, &var_count, input_vars.data());

    for (size_t i = 0; i < input_vars.size(); i++) {
        LOG->info("NAME: {}", input_vars.at(i)->name);
    }

    uint32_t set_count = 0;
    SpvReflectResult descriptor_set_result = spvReflectEnumerateDescriptorSets(&shader_module, &set_count, nullptr);
    if (descriptor_set_result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("[Shader] Failed to enumerate descriptor sets");
    }

    std::vector<SpvReflectDescriptorSet*> descriptor_sets(set_count);
    descriptor_set_result = spvReflectEnumerateDescriptorSets(&shader_module, &set_count, descriptor_sets.data());
    if (descriptor_set_result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("[Shader] Failed to enumerate descriptor sets");
    }

    std::unordered_map<uint32_t, ShaderDescriptorSet> sets;
    for (SpvReflectDescriptorSet* set : descriptor_sets) {
        std::vector<ShaderDescriptorBinding> bindings;
        for (uint32_t b = 0; b < set->binding_count; b++) {
            SpvReflectDescriptorBinding* reflect_binding = set->bindings[b];
            ShaderDescriptorType type = convert_descriptor_type(reflect_binding->descriptor_type);

            if (reflect_binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                std::optional<ShaderBindingBlock> block = create_binding_block(reflect_binding->block);
                ShaderDescriptorBinding binding(reflect_binding->name, reflect_binding->binding, type, block);
                bindings.push_back(std::move(binding));
            }
            else {
                ShaderDescriptorBinding binding(reflect_binding->name, reflect_binding->binding, type, {});
                bindings.push_back(std::move(binding));
            }
        }
        ShaderDescriptorSet descriptor_set(set->set, std::move(bindings));
        uint32_t set_num = descriptor_set.set();
        sets.insert({ set_num, std::move(descriptor_set) });
    }

    if (input_enum_result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("[Shader] Failed to enumerate input variables");
    }

    m_reflection_data.sets = std::move(sets);
}
const std::unordered_map<uint32_t, ShaderDescriptorSet>& Shader::descriptor_sets() const
{
    return m_reflection_data.sets;
}

const bool Shader::has_descriptor_set(uint32_t set) const
{
    return m_reflection_data.sets.contains(set);
}
const ShaderDescriptorSet& Shader::descriptor_set(uint32_t set) const
{
    return m_reflection_data.sets.at(set);
}

ShaderDescriptorSet::ShaderDescriptorSet(uint32_t set, std::vector<ShaderDescriptorBinding> bindings)
    : m_set(set)
    , m_bindings(std::move(bindings))
{
}
uint32_t ShaderDescriptorSet::set() const
{
    return m_set;
}

const ShaderDescriptorBinding& ShaderDescriptorSet::binding(uint32_t binding) const
{
    return m_bindings.at(binding);
}

const std::vector<ShaderDescriptorBinding>& ShaderDescriptorSet::bindings() const
{
    return m_bindings;
}
uint32_t ShaderDescriptorSet::binding_count() const
{
    return m_bindings.size();
}

ShaderDescriptorBinding::ShaderDescriptorBinding(
    std::string name, uint32_t binding, ShaderDescriptorType type, std::optional<ShaderBindingBlock> block)
    : m_name(std::move(name))
    , m_binding(binding)
    , m_type(type)
    , m_block(block)
{
}
std::string ShaderDescriptorBinding::name() const
{
    return m_name;
}
uint32_t ShaderDescriptorBinding::binding() const
{
    return m_binding;
}
const ShaderDescriptorType& ShaderDescriptorBinding::type() const
{
    return m_type;
}
const ShaderBindingBlock& ShaderDescriptorBinding::block() const
{
    return m_block.value();
}
const ShaderBindingBlock& ShaderDescriptorBinding::member(const std::string& name) const
{
    return m_block.value().member(name);
}
const std::unordered_map<std::string, ShaderBindingBlock>& ShaderDescriptorBinding::members() const
{
    return m_block.value().members();
}

ShaderBindingBlock::ShaderBindingBlock(
    std::string name, uint32_t size, uint32_t offset, std::unordered_map<std::string, ShaderBindingBlock> members)
    : m_name(name)
    , m_size(size)
    , m_offset(offset)
    , m_members(std::move(members))
{
}

const ShaderBindingBlock& ShaderBindingBlock::member(const std::string& name) const
{
    return m_members.at(name);
}
std::string ShaderBindingBlock::name() const
{
    return m_name;
}
uint32_t ShaderBindingBlock::size() const
{
    return m_size;
}
uint32_t ShaderBindingBlock::offset() const
{
    return m_offset;
}

const std::unordered_map<std::string, ShaderBindingBlock>& ShaderBindingBlock::members() const
{
    return m_members;
}
UniformLocation ShaderBindingBlock::location() const
{
    return UniformLocation(offset());
}

}