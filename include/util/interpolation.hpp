#pragma once

#include <glm/glm.hpp>

#include <array>

inline static float trilerp(const std::array<float, 8> v, const float x, const float y,
    const float z)
{
    float accumulated = 0.f;
    for (size_t it = 0; it < v.size(); ++it)
    {
        const size_t i = (it & 4) >> 2;
        const size_t j = (it & 2) >> 1;
        const size_t k = (it & 1) >> 0;
        accumulated +=
            (i * x + (1 - i) * (1 - x)) *
            (j * y + (1 - j) * (1 - y)) *
            (k * z + (1 - k) * (1 - z)) * v[it];
    }
    return accumulated;
}