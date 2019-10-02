#include <texture/noise.hpp>

color noise_texture::value_at(const float u, const float v, const position& p) const
{
    return this->noise.noise(p) * color{ 1.f };
}