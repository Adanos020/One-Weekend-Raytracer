#include <texture/noise.hpp>

noise_texture::noise_texture(const float scale, const noise_transform_fn& noise_transform)
    : scale(scale)
    , noise_transform(noise_transform)
{
}

color noise_texture::value_at(const float u, const float v, const position& p) const
{
    return color{ 1.f } * this->noise_transform(this->noise, this->scale * p);
}