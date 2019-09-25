#include <world.hpp>

hit_record_opt world::hit(const ray& r, const float t_min, const float t_max) const
{
    float closest = t_max;
    hit_record_opt result;
    for (const unique_hittable& object : this->hittables)
    {
        if (const hit_record_opt hit_rec = object->hit(r, t_min, closest))
        {
            closest = hit_rec->t;
            result = hit_rec;
        }
    }
    return result;
}