#include <material/diffuse_light.hpp>

#include <texture/constant.hpp>

diffuse_light::diffuse_light(const color& emit)
    : emit(std::make_unique<constant_texture>(emit))
{
}

diffuse_light::diffuse_light(unique_texture&& emit)
    : emit(std::move(emit))
{
}

scattering_opt diffuse_light::scatter(const line& ray, const struct hit_record& hit) const
{
    return {};
}

color diffuse_light::emitted(const std::pair<float, float> uv, const position& p) const
{
    return this->emit->value_at(uv, p);
}