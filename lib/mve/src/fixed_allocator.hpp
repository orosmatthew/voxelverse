#pragma once

namespace mve::detail {

template <typename T>
class FixedAllocator {
public:
    template <typename... Args>
    Handle allocate(Args&&... args)
    {
        if (!m_free_indices.empty()) {
            size_t index = m_free_indices.back();
            m_free_indices.pop_back();
            m_data[index] = T { std::forward<Args>(args)... };
            return index + 1;
        }
        m_data.emplace_back(T { std::forward<Args>(args)... });
        return m_data.size();
    }

    void free(const Handle handle)
    {
        MVE_VAL_ASSERT(
            handle >= 1 && handle <= m_data.size() && m_data.at(handle - 1).has_value(),
            "[FixedAllocator] Attempt to free invalid handle");
        m_data[handle - 1].reset();
        m_free_indices.push_back(handle - 1);
    }

    T& get(const Handle handle)
    {
        MVE_VAL_ASSERT(
            handle >= 1 && handle <= m_data.size() && m_data.at(handle - 1).has_value(),
            "[FixedAllocator] Attempt to get invalid handle");
        return *m_data[handle - 1];
    }

    const T& get(const Handle handle) const
    {
        MVE_VAL_ASSERT(
            handle >= 1 && handle <= m_data.size() && m_data.at(handle - 1).has_value(),
            "[FixedAllocator] Attempt to get invalid handle");
        return *m_data[handle - 1];
    }

    const std::vector<std::optional<T>>& data() const
    {
        return m_data;
    }

    std::vector<std::optional<T>>& data()
    {
        return m_data;
    }

private:
    std::vector<std::optional<T>> m_data;
    std::vector<size_t> m_free_indices;
};

}