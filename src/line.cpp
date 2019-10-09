#include <line.hpp>

#include <material.hpp>
#include <util/random.hpp>
#include <render_plan.hpp>

#include <glm/gtc/constants.hpp>

#include <cfloat>

position line::point_at_parameter(const float t) const
{
    return origin + t * direction;
}

color line::seen_color(const scene& world, const int32_t depth) const
{
    if (const hit_record_opt hit = world.hit(*this, min_max<float>{ 0.0001f, FLT_MAX }))
    {
        if (hit->p_material)
        {
            const color emitted = hit->p_material->emitted(hit->uv, hit->point);
            if (depth < 50)
            {
                if (const scattering_opt s = hit->p_material->scatter(*this, *hit))
                {
                    return emitted + s->attenuation * s->scattered_ray.seen_color(world, depth + 1);
                }
            }
            return emitted;
        }
    }
    const displacement norm_direction = this->direction / glm::length(this->direction);
    const float u = 1.f - (glm::atan(norm_direction.z, norm_direction.x) + glm::pi<float>()) * glm::one_over_two_pi<float>();
    const float v = (glm::asin(norm_direction.y) + glm::half_pi<float>()) * glm::one_over_pi<float>();
    return world.sky->value_at(std::make_pair(u, v), this->origin + this->direction);
}