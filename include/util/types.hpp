#pragma once

#include <glm/glm.hpp>

#include <utility>

using position = glm::vec3;
using displacement = glm::vec3;
using color = glm::vec3;
using axis = glm::vec3;

template <typename T>
struct min_max
{
    T min;
    T max;

    min_max(const T& min, const T& max)
        : min(min), max(max)
    {
    }

    min_max(const std::pair<const T&, const T&>& pair)
        : min(pair.first), max(pair.second)
    {
    }
};

template <typename T>
struct iterator_pair
{
    typename T::iterator begin;
    typename T::iterator end;
};

template <typename T>
struct const_iterator_pair
{
    typename T::const_iterator begin;
    typename T::const_iterator end;
};