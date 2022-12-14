#pragma once

#include <vector>

namespace mve {

/**
 * @brief Types of bindings for descriptor set
 */
enum DescriptorType {
    e_uniform_buffer,
};

/**
 * @brief Class that represents the types that are in a descriptor set
 */
class DescriptorSetLayout {
public:
    /**
     * @brief Construct a descriptor set layout
     */
    DescriptorSetLayout();

    /**
     * @brief Add a binding
     * @param type - Type associated with the binding
     * @return - Returns binding number (referenced in shaders)
     */
    uint32_t push_back(DescriptorType type);

    /**
     * @brief Get number of bindings in the descriptor set layout
     * @return - Returns number of bindings
     */
    [[nodiscard]] size_t size() const;

    /**
     * @brief Get type of binding
     * @param binding - binding location
     * @return - Returns type of binding
     */
    [[nodiscard]] DescriptorType type_at(uint32_t binding) const;

    /**
     * @brief Clears the layout for re-use
     */
    void clear();

private:
    std::vector<DescriptorType> m_bindings;
};

}