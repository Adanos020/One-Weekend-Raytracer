#pragma once

#include <material.hpp>
#include <texture.hpp>
#include <util/types.hpp>

class lambertian : public material
{
public:
    lambertian(unique_texture&& albedo);
    virtual scattering_opt scatter(const ray&, const struct hit_record&) const override;

public:
    unique_texture albedo;
};