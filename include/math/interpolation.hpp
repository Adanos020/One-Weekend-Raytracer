#pragma once

#include <glm/glm.hpp>

#include <array>

inline static float trilerp(const std::array<float, 8>& values,
    const float u, const float v, const float w)
{
    float accumulated = 0.f;
    for (size_t it = 0; it < values.size(); ++it)
    {
        const size_t i = (it & 4) >> 2;
        const size_t j = (it & 2) >> 1;
        const size_t k = (it & 1) >> 0;
        accumulated +=
            (i * u + (1 - i) * (1 - u)) *
            (j * v + (1 - j) * (1 - v)) *
            (k * w + (1 - k) * (1 - w)) * values[it];
    }
    return accumulated;
}

inline static float perlin_trilerp(const std::array<glm::vec3, 8>& values,
    const float u, const float v, const float w)
{
    const float uu = u * u * (3.f - 2.f * u);
    const float vv = v * v * (3.f - 2.f * v);
    const float ww = w * w * (3.f - 2.f * w);

    float accumulated = 0.f;
    for (size_t it = 0; it < values.size(); ++it)
    {
        const size_t i = (it & 4) >> 2;
        const size_t j = (it & 2) >> 1;
        const size_t k = (it & 1) >> 0;
        const glm::vec3 weight = { u - i, v - j, w - k };
        accumulated +=
            (i * uu + (1 - i) * (1 - uu)) *
            (j * vv + (1 - j) * (1 - vv)) *
            (k * ww + (1 - k) * (1 - ww)) * glm::dot(values[it], weight);
    }
    return accumulated;
}