#pragma once

#include <material.hpp>

class metal : public material
{
public:
    metal(const color& albedo, const float fuzz);
    virtual scattering_opt scatter(const ray&, const struct hit_record&) const override;

public:
    color albedo;
    float fuzz;
};