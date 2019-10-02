#include <texture/constant.hpp>

constant_texture::constant_texture(const color& value)
    : value(value)
{
}

color constant_texture::value_at(const float u, const float v, const position& p) const
{
    return this->value;
}
