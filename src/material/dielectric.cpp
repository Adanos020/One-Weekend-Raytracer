#include <material/dielectric.hpp>

#include <hittable.hpp>
#include <random.hpp>

dielectric::dielectric(const float refractive_index, const color attenuation)
    : refractive_index(refractive_index)
    , attenuation(attenuation)
{
}

scattering_opt dielectric::scatter(const ray& r, const hit_record& hit) const
{
    const float direction_dot_normal = glm::dot(r.direction, hit.normal);
    const float direction_length = glm::length(r.direction);

    displacement outward_normal;
    float eta;
    float cosine;

    if (direction_dot_normal > 0.f)
    {
        outward_normal = -hit.normal;
        eta = this->refractive_index;
        cosine = this->refractive_index * direction_dot_normal / direction_length;
    }
    else
    {
        outward_normal = hit.normal;
        eta = 1.f / this->refractive_index;
        cosine = -direction_dot_normal / direction_length;
    }

    const displacement refracted = glm::refract(glm::normalize(r.direction), outward_normal, eta);
    const displacement reflected = glm::reflect(r.direction, hit.normal);

    const float reflect_probability = refracted != displacement{ 0.f }
        ? schlick(cosine, this->refractive_index)
        : 1.f;

    if (random_chance(reflect_probability))
    {
        return scattering{ attenuation, ray{ hit.point, reflected } };
    }
    return scattering{ attenuation, ray{ hit.point, refracted } };
}

float dielectric::schlick(const float cosine, const float refractive_index)
{
    float r0 = (1 - refractive_index) / (1 + refractive_index);
    r0 *= r0;
    return r0 + (1 - r0) * glm::pow(1 - cosine, 5);
}
