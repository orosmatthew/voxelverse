#include "descriptor_set_layout.hpp"

namespace mve {

DescriptorSetLayout::DescriptorSetLayout()
    : m_bindings(std::vector<DescriptorType>())
{
}

uint32_t DescriptorSetLayout::push_back(DescriptorType type)
{
    uint32_t binding = static_cast<uint32_t>(m_bindings.size());
    m_bindings.push_back(type);
    return binding;
}

void DescriptorSetLayout::clear()
{
    m_bindings.clear();
}

size_t DescriptorSetLayout::size() const
{
    return m_bindings.size();
}

DescriptorType DescriptorSetLayout::type_at(uint32_t binding) const
{
    return m_bindings.at(static_cast<size_t>(binding));
}

}