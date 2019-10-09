#include <texture/checker.hpp>

#include <texture/constant.hpp>

#include <stdexcept>

checker_texture::checker_texture(const float scale, const color& odd, const color& even)
    : scale(scale)
    , odd(std::make_unique<constant_texture>(odd))
    , even(std::make_unique<constant_texture>(even))
{
}

checker_texture::checker_texture(const float scale, unique_texture&& odd, unique_texture&& even)
    : scale(scale), odd(std::move(odd)), even(std::move(even))
{
    if (!this->odd || !this->even)
    {
        throw std::runtime_error{ "checker_texture: None of given textures should be null." };
    }
}

color checker_texture::value_at(const std::pair<float, float> uv, const position& p) const
{
    if (glm::sin(scale * p.x) * glm::sin(scale * p.y) * glm::sin(scale * p.z) < 0)
    {
        return this->odd->value_at(uv, p);
    }
    return this->even->value_at(uv, p);
}