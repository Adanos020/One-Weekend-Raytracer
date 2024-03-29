#include <material/lambertian.hpp>

#include <hittable.hpp>
#include <shape/ball.hpp>
#include <texture/constant.hpp>
#include <util/random.hpp>

#include <stdexcept>

lambertian::lambertian(const color& albedo)
    : albedo(std::make_unique<constant_texture>(albedo))
{
}

lambertian::lambertian(unique_texture&& albedo)
    : albedo(std::move(albedo))
{
    if (!this->albedo)
    {
        throw std::runtime_error{ "lambertian: Given texture should not be null." };
    }
}

scattering_opt lambertian::scatter(const line& ray, const hit_record& hit) const
{
    const position target = hit.point + hit.normal + random_direction();
    return scattering{
        this->albedo->value_at(hit.uv, hit.point),
        line{ hit.point, target - hit.point, ray.time }
    };
}