#include <material/diffuse_light.hpp>

diffuse_light::diffuse_light(unique_texture&& emit)
    : emit(std::move(emit))
{
}

scattering_opt diffuse_light::scatter(const ray& r, const struct hit_record& hit) const
{
    return {};
}

color diffuse_light::emitted(const std::pair<float, float> uv, const position& p) const
{
    return this->emit->value_at(uv, p);
}