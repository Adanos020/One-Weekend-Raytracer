#pragma once

#include <iterator>
#include <utility>

template <typename T>
struct min_max
{
    T min;
    T max;

    constexpr min_max(const T& min = T{}, const T& max = T{})
        : min(min), max(max)
    {
    }

    constexpr min_max(const std::pair<const T&, const T&>& pair)
        : min(pair.first), max(pair.second)
    {
    }
};

template <typename T>
struct from_to
{
    T from;
    T to;

    constexpr from_to(const T& from = T{}, const T& to = T{})
        : from(from), to(to)
    {
    }

    constexpr from_to(const std::pair<const T&, const T&>& pair)
        : from(pair.first), to(pair.second)
    {
    }
};

template <typename T>
struct iterator_pair
{
    typename T::iterator begin;
    typename T::iterator end;

    iterator_pair(typename T::iterator begin, typename T::iterator end)
        : begin(begin), end(end)
    {
    }

    iterator_pair(T& container)
        : iterator_pair(std::begin(container), std::end(container))
    {
    }
};

template <typename T>
struct const_iterator_pair
{
    typename T::const_iterator begin;
    typename T::const_iterator end;

    const_iterator_pair(typename T::const_iterator begin, typename T::const_iterator end)
        : begin(begin), end(end)
    {
    }

    const_iterator_pair(const T& container)
        : const_iterator_pair(std::cbegin(container), std::cend(container))
    {
    }
};