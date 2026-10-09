#pragma once

#include <cstddef>
#include <iterator>
#include <tuple>

#include "Zipper.hpp"

namespace engine::ecs {

template <typename... Components>
class IndexedZipper
{
public:
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::tuple<std::size_t, Components &...>;
        using reference = value_type;
        using pointer = void;
        using difference_type = std::ptrdiff_t;

        explicit Iterator(typename Zipper<Components...>::Iterator it) : m_it(it) {}

        value_type operator*() const { return std::tuple_cat(std::make_tuple(m_it.index()), *m_it); }

        Iterator &operator++()
        {
            ++m_it;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator copy = *this;
            ++(*this);
            return copy;
        }

        bool operator==(const Iterator &other) const { return m_it == other.m_it; }

    private:
        typename Zipper<Components...>::Iterator m_it;
    };

    explicit IndexedZipper(SparseArray<Components> &...arrays) : m_zipper(arrays...) {}

    Iterator begin() const { return Iterator(m_zipper.begin()); }
    Iterator end() const { return Iterator(m_zipper.end()); }

private:
    Zipper<Components...> m_zipper;
};

} // namespace engine::ecs
