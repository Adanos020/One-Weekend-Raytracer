#pragma once

#include <material.hpp>
#include <texture.hpp>

class metal : public material
{
public:
    metal(const color&, const float fuzz);
    metal(unique_texture&&, const float fuzz);
    virtual scattering_opt scatter(const line&, const struct hit_record&) const override;

public:
    unique_texture albedo;
    float fuzz;
};