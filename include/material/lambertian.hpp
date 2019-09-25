#pragma once

#include <material.hpp>
#include <types.hpp>

class lambertian : public material
{
public:
    lambertian(const color& albedo);
    virtual scattering_opt scatter(const ray&, const struct hit_record&) const override;

public:
    color albedo;
};