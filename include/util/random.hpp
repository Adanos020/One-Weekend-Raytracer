#pragma once

#include <util/types.hpp>

#include <chrono>
#include <random>
#include <type_traits>

inline static bool random_chance(const float probability)
{
    static const auto seed = uint32_t(std::chrono::system_clock::now().time_since_epoch().count());
    static std::default_random_engine rng{ seed };
    std::bernoulli_distribution distribution{ probability };
    return distribution(rng);
}

template <typename T>
inline static auto random_uniform(const T min = T(0), const T max = T(1))
{
    static_assert(std::is_arithmetic_v<T>);

    static const auto seed = uint32_t(std::chrono::system_clock::now().time_since_epoch().count());
    static std::default_random_engine rng{ seed };

    if constexpr (std::is_integral_v<T>)
    {
        std::uniform_int_distribution<T> distribution{ min, max };
        return distribution(rng);
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        std::uniform_real_distribution<T> distribution{ min, max };
        return distribution(rng);
    }
}

inline static color random_color()
{
    return color{ random_uniform(0.f, 1.f), random_uniform(0.f, 1.f), random_uniform(0.f, 1.f) };
}

inline static displacement random_direction()
{
    while (true)
    {
        const displacement dir = {
            random_uniform(-1.f, 1.f), random_uniform(-1.f, 1.f), random_uniform(-1.f, 1.f)
        };
        if (const float l = glm::length(dir); l * l < 1.f)
        {
            return dir;
        }
    }
}

inline static displacement random_in_unit_disk(const axis& ax = { 0.f, 0.f, 1.f })
{
    while (true)
    {
        if (const displacement dir = (axis{ 1.f } -ax) * random_direction();
            glm::dot(dir, dir) < 1.f)
        {
            return dir;
        }
    }
}