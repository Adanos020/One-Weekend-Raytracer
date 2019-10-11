#include <line.hpp>

#include <material.hpp>
#include <math/sphere.hpp>
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
    return world.sky->value_at(uv_on_sphere(glm::normalize(this->direction)), this->origin + this->direction);
}