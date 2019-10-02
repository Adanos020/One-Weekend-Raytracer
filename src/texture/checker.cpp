#include <texture/checker.hpp>

#include <stdexcept>

checker_texture::checker_texture(unique_texture&& odd, unique_texture&& even)
    : odd(std::move(odd)), even(std::move(even))
{
    if (!this->odd || !this->even)
    {
        throw std::runtime_error{ "checker_texture: None of given textures should be null." };
    }
}

color checker_texture::value_at(const float u, const float v, const position& p) const
{
    if (glm::sin(10.f * p.x) * glm::sin(10.f * p.y) * glm::sin(10.f * p.z) < 0)
    {
        return this->odd->value_at(u, v, p);
    }
    return this->even->value_at(u, v, p);
}