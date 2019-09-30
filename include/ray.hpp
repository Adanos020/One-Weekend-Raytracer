#pragma once

#include <util/types.hpp>

#include <optional>

struct ray
{
    position origin;
    displacement direction;

    position point_at_parameter(const float t) const;
    color seen_color(const class world&, const int32_t depth) const;
};