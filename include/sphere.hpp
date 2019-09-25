#pragma once

#include <hittable.hpp>
#include <material.hpp>
#include <types.hpp>

struct sphere : hittable
{
    position center;
    float radius;
    unique_material mat;

    sphere() = default;
    sphere(const position& center, const float radius, unique_material&&);

    virtual hit_record_opt hit(
        const struct ray&, const float t_min, const float t_max) const override;
};