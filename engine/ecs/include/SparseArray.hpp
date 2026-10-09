#pragma once

#include <cstddef>
#include <memory>
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
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    SparseArray() = default;
    SparseArray(const SparseArray &) = default;
    SparseArray(SparseArray &&) noexcept = default;
    ~SparseArray() = default;
    SparseArray &operator=(const SparseArray &) = default;
    SparseArray &operator=(SparseArray &&) noexcept = default;

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

    iterator begin() { return m_data.begin(); }
    const_iterator begin() const { return m_data.begin(); }
    const_iterator cbegin() const { return m_data.cbegin(); }
    iterator end() { return m_data.end(); }
    const_iterator end() const { return m_data.end(); }
    const_iterator cend() const { return m_data.cend(); }

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

    template <typename... Params>
    reference_type emplaceAt(size_type pos, Params &&...params)
    {
        growTo(pos);
        m_data[pos].emplace(std::forward<Params>(params)...);
        return m_data[pos];
    }

    void erase(size_type pos)
    {
        if (pos < m_data.size())
            m_data[pos].reset();
    }

    std::optional<size_type> getIndex(const_reference_type value) const
    {
        for (size_type i = 0; i < m_data.size(); ++i) {
            if (std::addressof(m_data[i]) == std::addressof(value))
                return i;
        }
        return std::nullopt;
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
