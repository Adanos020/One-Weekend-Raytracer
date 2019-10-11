#pragma once

#include <glm/glm.hpp>

#include <iterator>
#include <utility>

using position = glm::vec3;
using displacement = glm::vec3;
using color = glm::vec3;
using color_alpha = glm::vec4;
using rgb = glm::uvec3;
using rgba = glm::uvec4;

using axis = glm::vec3;
static constexpr axis x_axis = { 1.f, 0.f, 0.f };
static constexpr axis y_axis = { 0.f, 1.f, 0.f };
static constexpr axis z_axis = { 0.f, 0.f, 1.f };

template <typename T>
struct extent_2d
{
    static_assert(std::is_arithmetic_v<T>);

    constexpr extent_2d() = default;
    constexpr extent_2d(const T width, const T height)
        : width(width), height(height)
    {
    }

    constexpr float aspect() const
    {
        return float(this->width) / float(this->height);
    }

    T width = T{};
    T height = T{};
};

template <typename T>
struct extent_3d
{
    static_assert(std::is_arithmetic_v<T>);

    constexpr extent_3d() = default;
    constexpr extent_3d(const T width, const T height, const T depth)
        : width(width), height(height), depth(depth)
    {
    }

    T width;
    T height;
    T depth;
};

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