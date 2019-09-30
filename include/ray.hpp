#pragma once

#include <util/types.hpp>

#include <optional>

struct ray
{
    position origin;
    displacement direction;
    displacement inverse_direction;

    ray(const position& origin, const displacement& direction)
        : origin(origin), direction(direction), inverse_direction(1.f / direction)
    {
    }

    position point_at_parameter(const float t) const;
    color seen_color(const class world&, const int32_t depth) const;
};