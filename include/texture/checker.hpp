#pragma once

#include <texture.hpp>

class checker_texture : public texture
{
public:
    checker_texture() = default;
    checker_texture(unique_texture&& odd, unique_texture&& even);

    virtual color value_at(const float u, const float v, const position&) const override;

private:
    unique_texture odd;
    unique_texture even;
};