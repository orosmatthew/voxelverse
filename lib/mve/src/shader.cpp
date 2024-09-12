#include <mve/shader.hpp>

#include <format>
#include <fstream>
#include <optional>
#include <vector>

#include <spirv_reflect.h>

#include "./logger.hpp"
#include <mve/common.hpp>

namespace mve {

Shader::Shader(const std::filesystem::path& file_path)
{
    Logger::debug(std::format("[Shader] Loading shader: {}", file_path.string()));

    std::ifstream file(file_path, std::ios::binary);

    MVE_ASSERT(file.is_open(), "[Shader] Failed to open shader file: " + file_path.string())

    std::vector<uint32_t> contents;
    uint32_t data;
    while (file.read(reinterpret_cast<char*>(&data), sizeof(data))) {
        contents.push_back(data);
    }

    file.close();

    m_spv_code = contents;
    create_reflection_data();
}

std::vector<uint32_t> Shader::spv_code() const noexcept
{
    return m_spv_code;
}

static ShaderDescriptorType convert_descriptor_type(const SpvReflectDescriptorType type)
{
    switch (type) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return ShaderDescriptorType::uniform_buffer;
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return ShaderDescriptorType::combined_image_sampler;
    default:
        MVE_ASSERT(false, "[Shader] Failed to convert descriptor type")
    }
}

static std::optional<ShaderBindingBlock> create_binding_block( // NOLINT(*-no-recursion)
    const SpvReflectBlockVariable& reflect_block)
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
    MVE_ASSERT(shader_module_result == SPV_REFLECT_RESULT_SUCCESS, "[Shader] Failed to generate reflection data")

    uint32_t var_count = 0;
    SpvReflectResult input_enum_result = spvReflectEnumerateInputVariables(&shader_module, &var_count, nullptr);
    MVE_ASSERT(input_enum_result == SPV_REFLECT_RESULT_SUCCESS, "[Shader] Failed to enumerate input variables")

    std::vector<SpvReflectInterfaceVariable*> input_vars(var_count);
    input_enum_result = spvReflectEnumerateInputVariables(&shader_module, &var_count, input_vars.data());

    uint32_t set_count = 0;
    SpvReflectResult descriptor_set_result = spvReflectEnumerateDescriptorSets(&shader_module, &set_count, nullptr);
    MVE_ASSERT(descriptor_set_result == SPV_REFLECT_RESULT_SUCCESS, "[Shader] Failed to enumerate descriptor sets")

    std::vector<SpvReflectDescriptorSet*> descriptor_sets(set_count);
    descriptor_set_result = spvReflectEnumerateDescriptorSets(&shader_module, &set_count, descriptor_sets.data());
    MVE_ASSERT(descriptor_set_result == SPV_REFLECT_RESULT_SUCCESS, "[Shader] Failed to enumerate descriptor sets")

    std::unordered_map<uint32_t, ShaderDescriptorSet> sets;
    for (SpvReflectDescriptorSet* set : descriptor_sets) {
        std::unordered_map<uint32_t, ShaderDescriptorBinding> bindings;
        for (uint32_t b = 0; b < set->binding_count; b++) {
            SpvReflectDescriptorBinding* reflect_binding = set->bindings[b];
            ShaderDescriptorType type = convert_descriptor_type(reflect_binding->descriptor_type);

            if (reflect_binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                std::optional<ShaderBindingBlock> block = create_binding_block(reflect_binding->block);
                ShaderDescriptorBinding binding(reflect_binding->name, reflect_binding->binding, type, block);
                bindings.insert({ reflect_binding->binding, std::move(binding) });
            }
            else {
                ShaderDescriptorBinding binding(reflect_binding->name, reflect_binding->binding, type, {});
                bindings.insert({ reflect_binding->binding, std::move(binding) });
            }
        }
        ShaderDescriptorSet descriptor_set(set->set, std::move(bindings));
        uint32_t set_num = descriptor_set.set();
        sets.insert({ set_num, std::move(descriptor_set) });
    }

    MVE_ASSERT(input_enum_result == SPV_REFLECT_RESULT_SUCCESS, "[Shader] Failed to enumerate input variables")

    m_reflection_data.sets = std::move(sets);
}

const std::unordered_map<uint32_t, ShaderDescriptorSet>& Shader::descriptor_sets() const
{
    return m_reflection_data.sets;
}

bool Shader::has_descriptor_set(const uint32_t set) const
{
    return m_reflection_data.sets.contains(set);
}

const ShaderDescriptorSet& Shader::descriptor_set(const uint32_t set) const
{
    return m_reflection_data.sets.at(set);
}

}