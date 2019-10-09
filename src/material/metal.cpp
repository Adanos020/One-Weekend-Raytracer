#include <material/metal.hpp>

#include <hittable.hpp>
#include <texture/constant.hpp>
#include <util/random.hpp>

metal::metal(const color& albedo, const float fuzz)
    : albedo(std::make_unique<constant_texture>(albedo))
{
}

metal::metal(unique_texture&& albedo, const float fuzz)
    : albedo(std::move(albedo))
    , fuzz(fuzz)
{
}

scattering_opt metal::scatter(const ray& r, const hit_record& hit) const
{
    const displacement reflected = glm::reflect(glm::normalize(r.direction), hit.normal);
    const ray scattered = { hit.point, reflected + (fuzz * random_direction()), r.time };
    if (glm::dot(scattered.direction, hit.normal) > 0.f)
    {
        return scattering{ this->albedo->value_at(hit.uv, hit.point), scattered };
    }
    return {};
}
