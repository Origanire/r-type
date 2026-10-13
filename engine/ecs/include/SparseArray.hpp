#pragma once

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

#include "IComponentArray.hpp"

namespace engine::ecs {

template <typename Component>
class SparseArray : public IComponentArray
{
public:
    std::optional<Component> &operator[](std::size_t index)
    {
        growTo(index);
        return m_data[index];
    }

    const std::optional<Component> &operator[](std::size_t index) const
    {
        static const std::optional<Component> empty;
        return index < m_data.size() ? m_data[index] : empty;
    }

    std::size_t size() const
    {
        return m_data.size();
    }

    void insertAt(std::size_t index, Component component)
    {
        growTo(index);
        m_data[index] = std::move(component);
    }

    void erase(std::size_t index) override
    {
        if (index < m_data.size())
            m_data[index].reset();
    }

private:
    void growTo(std::size_t index)
    {
        if (index >= m_data.size())
            m_data.resize(index + 1);
    }

    std::vector<std::optional<Component>> m_data;
};

} // namespace engine::ecs
