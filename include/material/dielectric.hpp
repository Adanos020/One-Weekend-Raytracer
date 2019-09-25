#pragma once

#include <material.hpp>
#include <types.hpp>

class dielectric : public material
{
public:
    dielectric(const color& albedo, const float refractive_index);
    virtual scattering_opt scatter(const ray&, const struct hit_record&) const override;

public:
    float refractive_index;
    color albedo;

private:
    static float schlick(float cosine, float refractive_index);
};