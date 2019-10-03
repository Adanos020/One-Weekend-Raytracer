#include <world.hpp>

#include <bounding_volume_hierarchy/bounding_volume_hierarchy_node.hpp>
#include <material/dielectric.hpp>
#include <material/lambertian.hpp>
#include <material/metal.hpp>
#include <shape/sphere.hpp>
#include <texture/checker.hpp>
#include <texture/constant.hpp>
#include <texture/noise.hpp>
#include <util/noise.hpp>
#include <util/random.hpp>

hit_record_opt world::hit(const ray& r, const min_max<float> t) const
{
    static const bounding_volume_hierarchy_node bvh = { this->hittables, t };
    return bvh.hit(r, t);
}

axis_aligned_bounding_box_opt world::bounding_box(const min_max<float> t) const
{
    return this->box;
}

world world::random_world_balls()
{
    world w;

    // earth
    w.spawn_object<sphere>(position{ 0.f, -1000.f, 0.f }, 1000.f,
        std::make_unique<lambertian>(
            std::make_unique<checker_texture>(
                std::make_unique<constant_texture>(color{ 0.2f, 0.3f, 0.1f }),
                std::make_unique<constant_texture>(color{ 0.9f }))));

    // random smaller spheres
    for (int32_t a = -11; a < 11; ++a)
    {
        for (int32_t b = -11; b < 11; ++b)
        {
            if (const position center = position{ float(a), 0.2f, float(b) } +(0.3f * random_in_unit_disk(axis{ 0.f, 1.f, 0.f }));
                glm::distance(center, position{ 4.f, 0.2f, 0.f }) > 0.9f)
            {
                unique_material mat;
                if (const float choose_material = random_uniform<float>(); choose_material < 0.35f)
                {
                    // Matte
                    mat = std::make_unique<lambertian>(std::make_unique<constant_texture>(random_color()));
                }
                else if (choose_material < 0.7f)
                {
                    // Metal
                    mat = std::make_unique<metal>(color{ 0.5f } + (0.5f * random_color()), random_uniform(0.f, 0.5f));
                }
                else
                {
                    // Glass and gems
                    mat = std::make_unique<dielectric>(color{ 0.5f } + (0.5f * random_color()), random_uniform(1.5f, 2.5f));
                }

                if (random_chance(0.5f))
                {
                    w.spawn_object<sphere>(
                        from_to<position>(center + displacement{ 0.f, random_uniform(0.1f, 0.3f), 0.f }, center),
                        min_max<float>(0.f, 1.f), 0.2f, std::move(mat));
                }
                else
                {
                    w.spawn_object<sphere>(center, 0.2f, std::move(mat));
                }
            }
        }
    }

    // 3 bigger spheres
    w.spawn_object<sphere>(position{ 0.f, 1.f, 0.f }, 1.f, std::make_unique<dielectric>(color{ 1.f }, 1.5f));
    w.spawn_object<sphere>(position{ -4.f, 1.f, 0.f }, 1.f,
        std::make_unique<lambertian>(std::make_unique<constant_texture>(color{ 0.4f, 0.2f, 0.1f })));
    w.spawn_object<sphere>(position{ 4.f, 1.f, 0.f }, 1.f, std::make_unique<metal>(color{ 0.7f, 0.6f, 0.5f }, 0.f));

    return w;
}

world world::two_noise_spheres()
{
    world w;

    w.spawn_object<sphere>(position{ 0.f, -1000.f, 0.f }, 1000.f,
        std::make_unique<lambertian>(std::make_unique<noise_texture>(3.f,
            [](const perlin& n, const glm::vec3& p) { return turbulence(n, p); })));
    w.spawn_object<sphere>(position{ 0.f, 2.f, 0.f }, 2.f,
        std::make_unique<lambertian>(std::make_unique<noise_texture>(2.f,
            [](const perlin& n, const glm::vec3& p) { return 0.5f * (1.f + glm::sin(p.z + 10.f * turbulence(n, p))); })));

    return w;
}