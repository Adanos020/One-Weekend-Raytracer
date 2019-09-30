#pragma once

#include <hittable.hpp>
#include <material.hpp>
#include <util/types.hpp>

struct sphere : hittable
{
    position center;
    float radius;
    unique_material mat;

    sphere() = default;
    sphere(const position& center, const float radius, unique_material&&);

    virtual hit_record_opt hit(const struct ray&, const min_max<float> t) const override;
    virtual axis_aligned_bounding_box_opt bounding_box(const min_max<float> t) const override;
};