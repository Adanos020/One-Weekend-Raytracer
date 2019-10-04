#include <texture/constant.hpp>

constant_texture::constant_texture(const color& value)
    : value(value)
{
}

color constant_texture::value_at(const std::pair<float, float> uv, const position& p) const
{
    return this->value;
}
