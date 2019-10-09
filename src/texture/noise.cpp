#include <texture/noise.hpp>

noise_texture::noise_texture(const float scale, const color& albedo, const noise_transform_fn& noise_transform)
    : scale(scale)
    , albedo(albedo)
    , noise_transform(noise_transform)
{
}

color noise_texture::value_at(const std::pair<float, float> uv, const position& p) const
{
    return this->albedo * this->noise_transform(this->noise, this->scale * p);
}