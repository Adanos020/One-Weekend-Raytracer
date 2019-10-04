#include <material/lambertian.hpp>

#include <hittable.hpp>
#include <shape/sphere.hpp>
#include <util/random.hpp>

#include <stdexcept>

lambertian::lambertian(unique_texture&& albedo)
    : albedo(std::move(albedo))
{
    if (!this->albedo)
    {
        throw std::runtime_error{ "lambertian: Given texture should not be null." };
    }
}

scattering_opt lambertian::scatter(const ray& r, const hit_record& hit) const
{
    const position target = hit.point + hit.normal + random_direction();
    return scattering{
        this->albedo->value_at(hit.uv, hit.point),
        ray{ hit.point, target - hit.point, r.time }
    };
}