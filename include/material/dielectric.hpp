#pragma once

#include <material.hpp>

class dielectric : public material
{
public:
    dielectric(const float refractive_index, const color attenuation);

    virtual scattering_opt scatter(const ray&, const struct hit_record&) const override;

public:
    float refractive_index;
    color attenuation;

private:
    static float schlick(float cosine, float refractive_index);
};