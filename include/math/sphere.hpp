#pragma once

#include <util/types.hpp>

#include <glm/gtc/constants.hpp>

struct sphere
{
    position origin;
    float radius;
};

inline static std::pair<float, float> uv_on_sphere(const position& normalized_p)
{
    return {
        1.f - (glm::atan(normalized_p.z, normalized_p.x) + glm::pi<float>()) * glm::one_over_two_pi<float>(),
        (glm::asin(normalized_p.y) + glm::half_pi<float>()) * glm::one_over_pi<float>()
    };
}