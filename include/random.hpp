#pragma once

#include <types.hpp>

#include <chrono>
#include <random>

static bool random_chance(const float probability)
{
    static const auto seed = uint32_t(std::chrono::system_clock::now().time_since_epoch().count());
    static std::default_random_engine rng{ seed };
    std::bernoulli_distribution distribution{ probability };
    return distribution(rng);
}

static float random_uniform(const float min, const float max)
{
    static const auto seed = uint32_t(std::chrono::system_clock::now().time_since_epoch().count());
    static std::default_random_engine rng{ seed };
    std::uniform_real_distribution<float> distribution{ min, max };
    return distribution(rng);
}

static displacement random_direction()
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

static displacement random_in_unit_disk()
{
    while (true)
    {
        if (const displacement dir = { random_uniform(-1.f, 1.f), random_uniform(-1.f, 1.f), 0.f };
            glm::dot(dir, dir) < 1.f)
        {
            return dir;
        }
    }
}