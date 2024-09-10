#pragma once

namespace mve::detail {

template <typename T>
class ResourceArena {
public:
    template <typename... Args>
    Handle allocate(Args&&... args)
    {
        std::optional<size_t> empty_index;
        for (size_t i = 0; i < m_data.size(); ++i) {
            if (!m_data[i].has_value()) {
                empty_index = i;
                break;
            }
        }
        if (empty_index.has_value()) {
            m_data[*empty_index] = T { std::forward<Args>(args)... };
            return *empty_index + 1;
        }
        m_data.emplace_back(std::forward<Args>(args)...);
        return m_data.size();
    }

    void free(const Handle handle)
    {
        MVE_VAL_ASSERT(
            handle >= 1 && handle <= m_data.size() && m_data.at(handle - 1).has_value(),
            "[Resource Allocator] Attempt to free invalid handle");
        m_data[handle - 1].reset();
    }

    T& get(const Handle handle)
    {
        MVE_VAL_ASSERT(
            handle >= 1 && handle <= m_data.size() && m_data.at(handle - 1).has_value(),
            "[Resource Allocator] Attempt to get invalid handle");
        return *m_data[handle - 1];
    }

    const T& get(const Handle handle) const
    {
        MVE_VAL_ASSERT(
            handle >= 1 && handle <= m_data.size() && m_data.at(handle - 1).has_value(),
            "[Resource Allocator] Attempt to get invalid handle");
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
};

}