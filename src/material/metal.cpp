#include <material/metal.hpp>

#include <hittable.hpp>
#include <random.hpp>

metal::metal(const color& albedo, const float fuzz)
    : albedo(albedo)
    , fuzz(fuzz)
{
}

scattering_opt metal::scatter(const ray& r, const hit_record& hit) const
{
    const displacement reflected = glm::reflect(glm::normalize(r.direction), hit.normal);
    const ray scattered = { hit.point, reflected + (fuzz * random_direction()) };
    if (glm::dot(scattered.direction, hit.normal) > 0.f)
    {
        return scattering{ this->albedo, scattered };
    }
    return {};
}
