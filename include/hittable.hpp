#pragma once

#include <bounding_volume_hierarchy/axis_aligned_bounding_box.hpp>
#include <util/types.hpp>

#include <memory>
#include <optional>

struct hit_record
{
    float t;
    position point;
    displacement normal;
    class material* p_material;
};

using hit_record_opt = std::optional<hit_record>;

class hittable
{
public:
    virtual ~hittable() = default;
    virtual hit_record_opt hit(const struct ray&, const min_max<float> t) const = 0;
    virtual axis_aligned_bounding_box_opt bounding_box(const min_max<float> t) const = 0;
};

using unique_hittable = std::unique_ptr<hittable>;