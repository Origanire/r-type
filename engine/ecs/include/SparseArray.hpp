#pragma once

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace engine::ecs {

template <typename Component>
class SparseArray
{
public:
    using value_type = std::optional<Component>;
    using reference_type = value_type &;
    using const_reference_type = const value_type &;
    using container_t = std::vector<value_type>;
    using size_type = typename container_t::size_type;

    reference_type operator[](size_type idx)
    {
        growTo(idx);
        return m_data[idx];
    }

    const_reference_type operator[](size_type idx) const
    {
        static const value_type empty;
        return idx < m_data.size() ? m_data[idx] : empty;
    }

    size_type size() const { return m_data.size(); }

    reference_type insertAt(size_type pos, const Component &component)
    {
        growTo(pos);
        m_data[pos] = component;
        return m_data[pos];
    }

    reference_type insertAt(size_type pos, Component &&component)
    {
        growTo(pos);
        m_data[pos] = std::move(component);
        return m_data[pos];
    }

    void erase(size_type pos)
    {
        if (pos < m_data.size())
            m_data[pos].reset();
    }

private:
    void growTo(size_type idx)
    {
        if (idx >= m_data.size())
            m_data.resize(idx + 1);
    }

    container_t m_data;
};

} // namespace engine::ecs
