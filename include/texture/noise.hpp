#pragma once

#include <texture.hpp>
#include <util/noise.hpp>

class noise_texture : public texture
{
public:
    noise_texture() = default;

    virtual color value_at(const float u, const float v, const position&) const override;

private:
    perlin noise;
};