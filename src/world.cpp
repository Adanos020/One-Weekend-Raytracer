#include <world.hpp>

#include <bounding_volume_hierarchy/bounding_volume_hierarchy_node.hpp>
#include <material/dielectric.hpp>
#include <material/lambertian.hpp>
#include <material/metal.hpp>
#include <util/random.hpp>
#include <sphere.hpp>

world world::random_world_balls()
{
    world w;

    // earth
    w.spawn_object<sphere>(position{ 0.f, -1000.f, 0.f }, 1000.f, std::make_unique<lambertian>(color{ 0.5f }));

    // random smaller spheres
    for (int32_t a = -11; a < 11; ++a)
    {
        for (int32_t b = -11; b < 11; ++b)
        {
            if (const position center = position{ float(a), 0.2f, float(b) } + (0.75f * random_in_unit_disk(axis{ 0.f, 1.f, 0.f }));
                glm::distance(center, position{ 4.f, 0.2f, 0.f }) > 0.9f)
            {
                unique_material mat;
                if (const float choose_material = random_uniform<float>(); choose_material < 0.2f)
                {
                    // Matte
                    mat = std::make_unique<lambertian>(random_color());
                }
                else if (choose_material < 0.5f)
                {
                    // Metal
                    mat = std::make_unique<metal>(color{ 0.5f } + (0.5f * random_color()), random_uniform(0.f, 0.5f));
                }
                else if (choose_material < 0.75f)
                {
                    // Glass and gems
                    mat = std::make_unique<dielectric>(color{ 0.5f } + (0.5f * random_color()), random_uniform(1.5f, 2.5f));
                }
                else
                {
                    // Hollow glass and gem balls
                    const float refractive_index = random_uniform(1.5f, 2.5f);
                    mat = std::make_unique<dielectric>(color{ 0.5f } + (0.5f * random_color()), refractive_index);
                    w.spawn_object<sphere>(center, -0.19f, std::make_unique<dielectric>(color{ 1.f }, refractive_index));
                }
                w.spawn_object<sphere>(center, 0.2f, std::move(mat));
            }
        }
    }

    // 4 bigger spheres
    w.spawn_object<sphere>(position{ 0.f, 1.f, 0.f }, 1.f, std::make_unique<dielectric>(color{ 1.f }, 1.5f));
    w.spawn_object<sphere>(position{ 0.f, 1.f, 3.f }, 1.f, std::make_unique<dielectric>(color{ 0.3f, 0.3f, 1.f }, 1.5f));
    w.spawn_object<sphere>(position{ 0.f, 1.f, 3.f }, -0.95f, std::make_unique<dielectric>(color{ 1.f }, 1.5f));
    w.spawn_object<sphere>(position{ -4.f, 1.f, 0.f }, 1.f, std::make_unique<lambertian>(color{ 0.4f, 0.2f, 0.1f }));
    w.spawn_object<sphere>(position{ 4.f, 1.f, 0.f }, 1.f, std::make_unique<metal>(color{ 0.7f, 0.6f, 0.5f }, 0.f));

    return w;
}

hit_record_opt world::hit(const ray& r, const min_max<float> t) const
{
    float closest = t.max;
    hit_record_opt result;
    for (const unique_hittable& object : this->hittables)
    {
        if (const hit_record_opt hit_rec = object->hit(r, min_max<float>{ t.min, closest }))
        {
            closest = hit_rec->t;
            result = hit_rec;
        }
    }
    return result;
//     static const bounding_volume_hierarchy_node bvh = { { this->hittables.begin(), this->hittables.end() }, { 0.f, 0.f } };
//     return bvh.hit(r, t);
}

axis_aligned_bounding_box_opt world::bounding_box(const min_max<float> t) const
{
    return this->box;
}