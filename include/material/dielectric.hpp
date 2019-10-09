#pragma once

#include <material.hpp>
#include <texture.hpp>
#include <util/types.hpp>

class dielectric : public material
{
public:
    dielectric(const color&, const float refractive_index);
    dielectric(unique_texture&&, const float refractive_index);
    virtual scattering_opt scatter(const ray&, const struct hit_record&) const override;

public:
    float refractive_index;
    unique_texture albedo;

private:
    static float schlick(float cosine, float refractive_index);
};