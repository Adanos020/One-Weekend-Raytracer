#include <material/lambertian.hpp>

#include <hittable.hpp>
#include <util/random.hpp>

lambertian::lambertian(const color& albedo)
    : albedo(albedo)
{
}

scattering_opt lambertian::scatter(const ray& r, const hit_record& hit) const
{
    const position target = hit.point + hit.normal + random_direction();
    return scattering{ this->albedo, ray{ hit.point, target - hit.point } };
}