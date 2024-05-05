#include <mve/detail/descriptor_set_allocator.hpp>

#include <mve/common.hpp>

namespace mve::detail {

DescriptorSetAllocator::DescriptorSetAllocator()
    : m_sizes({ { vk::DescriptorType::eSampler, 0.5f },
                { vk::DescriptorType::eCombinedImageSampler, 4.0f },
                { vk::DescriptorType::eSampledImage, 4.0f },
                { vk::DescriptorType::eStorageImage, 1.0f },
                { vk::DescriptorType::eUniformTexelBuffer, 1.0f },
                { vk::DescriptorType::eStorageTexelBuffer, 1.0f },
                { vk::DescriptorType::eUniformBuffer, 2.0f },
                { vk::DescriptorType::eStorageBuffer, 2.0f },
                { vk::DescriptorType::eUniformBufferDynamic, 1.0f },
                { vk::DescriptorType::eStorageBufferDynamic, 1.0f },
                { vk::DescriptorType::eInputAttachment, 0.5f } })
    , m_max_sets_per_pool(1000)
    , m_id_count(0)
    , m_current_pool_index(0)
{
}

void DescriptorSetAllocator::cleanup(const vk::DispatchLoaderDynamic& loader, const vk::Device device)
{
    for (std::optional<DescriptorSetImpl>& set : m_descriptor_sets) {
        if (set.has_value()) {
            device.freeDescriptorSets(set->vk_pool, 1, &set->vk_handle, loader);
        }
    }
    for (const vk::DescriptorPool descriptor_pool : m_descriptor_pools) {
        device.destroy(descriptor_pool, nullptr, loader);
    }
}

void DescriptorSetAllocator::free(
    const vk::DispatchLoaderDynamic& loader, const vk::Device device, const DescriptorSetImpl& descriptor_set)
{
    // ReSharper disable once CppExpressionWithoutSideEffects
    device.freeDescriptorSets(descriptor_set.vk_pool, 1, &descriptor_set.vk_handle, loader);
    m_descriptor_sets[descriptor_set.id].reset();
}

DescriptorSetImpl DescriptorSetAllocator::create(
    const vk::DispatchLoaderDynamic& loader, const vk::Device device, const vk::DescriptorSetLayout layout)
{
    if (m_descriptor_pools.empty()) {
        m_descriptor_pools.push_back(create_pool(loader, device, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet));
        m_current_pool_index = 0;
    }

    std::optional<vk::DescriptorSet> descriptor_set
        = try_create(loader, m_descriptor_pools.at(m_current_pool_index), device, layout);

    if (!descriptor_set.has_value()) {
        for (size_t i = 0; i < m_descriptor_pools.size(); i++) {
            if (i == m_current_pool_index) {
                continue;
            }
            descriptor_set = try_create(loader, m_descriptor_pools.at(i), device, layout);
            if (descriptor_set.has_value()) {
                m_current_pool_index = i;
                break;
            }
        }

        if (!descriptor_set.has_value()) {
            m_descriptor_pools.push_back(
                create_pool(loader, device, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet));
            m_current_pool_index = m_descriptor_pools.size() - 1;
            descriptor_set = try_create(loader, m_descriptor_pools.at(m_current_pool_index), device, layout);

            MVE_ASSERT(descriptor_set.has_value(), "[Renderer] Failed to allocate descriptor set")
        }
    }
    std::optional<size_t> id;
    for (size_t i = 0; i < m_descriptor_sets.size(); i++) {
        if (!m_descriptor_sets[i].has_value()) {
            id = i;
            break;
        }
    }
    if (!id.has_value()) {
        id = m_descriptor_sets.size();
        m_descriptor_sets.emplace_back();
    }
    DescriptorSetImpl descriptor_set_impl {
        .id = *id, .vk_handle = *descriptor_set, .vk_pool = m_descriptor_pools.at(m_current_pool_index)
    };
    m_descriptor_sets[*id] = descriptor_set_impl;
    return descriptor_set_impl;
}

std::optional<vk::DescriptorSet> DescriptorSetAllocator::try_create(
    const vk::DispatchLoaderDynamic& loader,
    const vk::DescriptorPool pool,
    const vk::Device device,
    const vk::DescriptorSetLayout layout)
{
    const auto alloc_info
        = vk::DescriptorSetAllocateInfo().setDescriptorPool(pool).setDescriptorSetCount(1).setPSetLayouts(&layout);

    if (vk::ResultValue<std::vector<vk::DescriptorSet>> descriptor_sets_result
        = device.allocateDescriptorSets(alloc_info, loader);
        descriptor_sets_result.result == vk::Result::eErrorOutOfPoolMemory
        || descriptor_sets_result.result == vk::Result::eErrorFragmentedPool) {
        return {};
    }
    else {
        if (descriptor_sets_result.result == vk::Result::eSuccess) {
            return descriptor_sets_result.value.at(0);
        }
        MVE_ASSERT(false, "[Renderer] Failed to allocate descriptor sets")
    }
}

vk::DescriptorPool DescriptorSetAllocator::create_pool(
    const vk::DispatchLoaderDynamic& loader, const vk::Device device, const vk::DescriptorPoolCreateFlags flags) const
{
    std::vector<vk::DescriptorPoolSize> sizes;
    sizes.reserve(m_sizes.size());

    std::ranges::transform(m_sizes, std::back_inserter(sizes), [&](const std::pair<vk::DescriptorType, float>& s) {
        return vk::DescriptorPoolSize(
            s.first, static_cast<uint32_t>(s.second * static_cast<float>(m_max_sets_per_pool)));
    });

    const auto pool_info
        = vk::DescriptorPoolCreateInfo().setFlags(flags).setMaxSets(m_max_sets_per_pool).setPoolSizes(sizes);

    const vk::ResultValue<vk::DescriptorPool> descriptor_pool_result
        = device.createDescriptorPool(pool_info, nullptr, loader);
    MVE_ASSERT(descriptor_pool_result.result == vk::Result::eSuccess, "[Renderer] Failed to create descriptor pool")
    return descriptor_pool_result.value;
}

}