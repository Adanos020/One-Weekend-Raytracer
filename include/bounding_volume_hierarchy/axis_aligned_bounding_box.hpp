#pragma once

#include <util/types.hpp>

#include <optional>

struct axis_aligned_bounding_box
{
public:
    position min{ 0.f };
    position max{ 0.f };

    static axis_aligned_bounding_box surrounding(const axis_aligned_bounding_box&, const axis_aligned_bounding_box&);

    bool hit(const struct ray&, min_max<float> t) const;
};

using axis_aligned_bounding_box_opt = std::optional<axis_aligned_bounding_box>;