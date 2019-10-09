#pragma once

#include <material.hpp>
#include <texture.hpp>
#include <util/types.hpp>

class lambertian : public material
{
public:
    lambertian(const color&);
    lambertian(unique_texture&&);
    virtual scattering_opt scatter(const line&, const struct hit_record&) const override;

public:
    unique_texture albedo;
};