#pragma once

#include <mve/include_vulkan.hpp>

#include <mve/detail/defs.hpp>

namespace mve::detail {
class DescriptorSetAllocator {
public:
    DescriptorSetAllocator();

    void cleanup(const vk::DispatchLoaderDynamic& loader, vk::Device device);

    void free(const vk::DispatchLoaderDynamic& loader, vk::Device device, const DescriptorSetImpl& descriptor_set);

    DescriptorSetImpl create(
        const vk::DispatchLoaderDynamic& loader, vk::Device device, vk::DescriptorSetLayout layout);

private:
    std::vector<std::pair<vk::DescriptorType, float>> m_sizes;
    uint32_t m_max_sets_per_pool;

    uint64_t m_id_count;
    std::vector<vk::DescriptorPool> m_descriptor_pools {};
    std::vector<std::optional<DescriptorSetImpl>> m_descriptor_sets {};
    size_t m_current_pool_index;

    static std::optional<vk::DescriptorSet> try_create(
        const vk::DispatchLoaderDynamic& loader,
        vk::DescriptorPool pool,
        vk::Device device,
        vk::DescriptorSetLayout layout);

    [[nodiscard]] vk::DescriptorPool create_pool(
        const vk::DispatchLoaderDynamic& loader,
        vk::Device device,
        vk::DescriptorPoolCreateFlags flags = vk::DescriptorPoolCreateFlags()) const;
};
}