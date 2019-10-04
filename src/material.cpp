#include <material.hpp>

color material::emitted(const std::pair<float, float>, const position&) const
{
    return color{};
}