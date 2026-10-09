#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <tuple>

#include "SparseArray.hpp"

namespace engine::ecs {

template <typename... Components>
class Zipper
{
    static_assert(sizeof...(Components) > 0, "Zipper needs at least one component type");

public:
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::tuple<Components &...>;
        using reference = value_type;
        using pointer = void;
        using difference_type = std::ptrdiff_t;

        Iterator(std::tuple<SparseArray<Components> *...> arrays, std::size_t idx, std::size_t max)
            : m_arrays(arrays), m_idx(idx), m_max(max)
        {
            skipIncomplete();
        }

        value_type operator*() const
        {
            return std::apply([this](auto *...arrays) { return value_type(*(*arrays)[m_idx]...); }, m_arrays);
        }

        Iterator &operator++()
        {
            ++m_idx;
            skipIncomplete();
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator copy = *this;
            ++(*this);
            return copy;
        }

        bool operator==(const Iterator &other) const { return m_idx == other.m_idx; }

        std::size_t index() const { return m_idx; }

    private:
        bool allPresent() const
        {
            return std::apply([this](auto *...arrays) { return ((*arrays)[m_idx].has_value() && ...); }, m_arrays);
        }

        void skipIncomplete()
        {
            while (m_idx < m_max && !allPresent())
                ++m_idx;
        }

        std::tuple<SparseArray<Components> *...> m_arrays;
        std::size_t m_idx;
        std::size_t m_max;
    };

    explicit Zipper(SparseArray<Components> &...arrays) : m_arrays(&arrays...), m_max(std::min({arrays.size()...})) {}

    Iterator begin() const { return Iterator(m_arrays, 0, m_max); }
    Iterator end() const { return Iterator(m_arrays, m_max, m_max); }

private:
    std::tuple<SparseArray<Components> *...> m_arrays;
    std::size_t m_max;
};

} // namespace engine::ecs
