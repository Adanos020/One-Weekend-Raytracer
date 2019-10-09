#pragma once

#include <material.hpp>
#include <texture.hpp>

class diffuse_light : public material
{
public:
    diffuse_light(const color&);
    diffuse_light(unique_texture&&);

    virtual scattering_opt scatter(const line&, const struct hit_record&) const override;
    virtual color emitted(const std::pair<float, float> uv, const position&) const override;

private:
    unique_texture emit;
};