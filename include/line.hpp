#pragma once

#include <util/colors.hpp>
#include <util/vector_types.hpp>

#include <optional>

struct line
{
    const position origin;
    const displacement direction;
    const displacement inverse_direction;
    const float time;

    line(const position& origin, const displacement& direction, const float time = 0.f)
        : origin(origin)
        , direction(direction)
        , inverse_direction(1.f / direction)
        , time(time)
    {
    }

    position point_at_parameter(const float t) const;
    color seen_color(const class scene&, const int32_t depth = 0) const;
};