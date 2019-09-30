#include <ray.hpp>

#include <material.hpp>
#include <util/random.hpp>
#include <world.hpp>

#include <cfloat>

position ray::point_at_parameter(const float t) const
{
    return origin + t * direction;
}

color ray::seen_color(const world& w, const int32_t depth) const
{
    if (const hit_record_opt hit = w.hit(*this, min_max<float>{ 0.0001f, FLT_MAX }))
    {
        if (hit->p_material && depth < 50)
        {
            if (const scattering_opt s = hit->p_material->scatter(*this, *hit))
            {
                return s->attenuation * s->scattered_ray.seen_color(w, depth + 1);
            }
        }
        return color{ 0.f };
    }
    const displacement unit_direction = glm::normalize(this->direction);
    const float t = 0.5f * (unit_direction.y + 1.f);
    return (1.f - t) * color{ 1.f, 1.f, 1.f } + t * color{ 0.5f, 0.7f, 1.f };
}