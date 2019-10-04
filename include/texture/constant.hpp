#pragma once

#include <texture.hpp>

class constant_texture : public texture
{
public:
    constant_texture() = default;
    constant_texture(const color&);

    virtual color value_at(const std::pair<float, float> uv, const position&) const override;

private:
    color value;
};