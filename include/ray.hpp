#pragma once

#include <util/types.hpp>

#include <optional>

struct ray
{
    const position origin;
    const displacement direction;
    const displacement inverse_direction;
    const float time;

    ray(const position& origin, const displacement& direction, const float time = 0.f)
        : origin(origin)
        , direction(direction)
        , inverse_direction(1.f / direction)
        , time(time)
    {
    }

    position point_at_parameter(const float t) const;
    color seen_color(const class world&, const int32_t depth) const;
};