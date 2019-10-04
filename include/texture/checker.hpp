#pragma once

#include <texture.hpp>

class checker_texture : public texture
{
public:
    checker_texture() = default;
    checker_texture(const float scale, unique_texture&& odd, unique_texture&& even);

    virtual color value_at(const std::pair<float, float> uv, const position&) const override;

private:
    float scale = 1.f;
    unique_texture odd;
    unique_texture even;
};