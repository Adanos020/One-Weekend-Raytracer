#include <material/dielectric.hpp>

#include <hittable.hpp>
#include <util/random.hpp>

dielectric::dielectric(const color& albedo, const float refractive_index)
    : refractive_index(refractive_index)
    , albedo(albedo)
{
}

scattering_opt dielectric::scatter(const ray& r, const hit_record& hit) const
{
    const float direction_dot_normal = glm::dot(r.direction, hit.normal);
    const float direction_length = glm::length(r.direction);

    const auto [outward_normal, eta, cosine] = direction_dot_normal > 0.f
        ? std::make_tuple(-hit.normal, this->refractive_index, this->refractive_index * direction_dot_normal / direction_length)
        : std::make_tuple(hit.normal, 1.f / this->refractive_index, -direction_dot_normal / direction_length);

    const displacement refracted = glm::refract(glm::normalize(r.direction), outward_normal, eta);
    const displacement reflected = glm::reflect(r.direction, hit.normal);

    const float reflect_probability = refracted != displacement{ 0.f }
        ? schlick(cosine, this->refractive_index)
        : 1.f;

    if (random_chance(reflect_probability))
    {
        return scattering{ albedo, ray{ hit.point, reflected, r.time } };
    }
    return scattering{ albedo, ray{ hit.point, refracted, r.time } };
}

float dielectric::schlick(const float cosine, const float refractive_index)
{
    const float r0 = glm::pow((1 - refractive_index) / (1 + refractive_index), 2);
    return r0 + (1 - r0) * glm::pow(1 - cosine, 5);
}
