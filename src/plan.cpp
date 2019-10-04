#include <plan.hpp>

#include <material/diffuse_light.hpp>
#include <material/dielectric.hpp>
#include <material/lambertian.hpp>
#include <material/metal.hpp>
#include <shape/sphere.hpp>
#include <texture/checker.hpp>
#include <texture/constant.hpp>
#include <texture/image.hpp>
#include <texture/noise.hpp>
#include <util/noise.hpp>
#include <util/random.hpp>

plan plan::random_balls(const extent_2d<uint32_t>& image_size)
{
    const camera cam = camera_create_info{
        position{ 8.f, 2.f, 3.f },
        position{ 0.f, 0.f, -1.f },
        axis{ 0.f, 1.f, 0.f },
        45.f,
        float(image_size.width) / float(image_size.height),
        0.05f,
        { 0.f, 1.f }
    };

    scene w;
    w.spawn_object<sphere>(position{ 0.f, -1000.f, 0.f }, 1000.f,
        std::make_unique<lambertian>(
            std::make_unique<checker_texture>(10.f,
                std::make_unique<constant_texture>(random_color()),
                std::make_unique<constant_texture>(random_color()))));

    for (int32_t a = -11; a < 11; ++a)
    {
        for (int32_t b = -11; b < 11; ++b)
        {
            if (const position center = position{ float(a), 0.2f, float(b) } + (0.3f * random_in_unit_disk(axis{ 0.f, 1.f, 0.f }));
                glm::distance(center, position{ 4.f, 0.2f, 0.f }) > 0.9f)
            {
                unique_material mat;
                if (const float choose_material = random_uniform<float>(); choose_material < 0.5f)
                {
                    unique_texture tex;
                    if (const float choose_texture = random_uniform<float>(); choose_texture < 0.25f)
                    {
                        tex = std::make_unique<checker_texture>(30.f,
                            std::make_unique<constant_texture>(random_color()),
                            std::make_unique<constant_texture>(random_color()));
                    }
                    else if (choose_texture < 0.5f)
                    {
                        tex = std::make_unique<constant_texture>(random_color());
                    }
                    else if (choose_texture < 0.75f)
                    {
                        tex = std::make_unique<noise_texture>(10.f,
                            [](const perlin& n, const glm::vec3& p) { return turbulence(n, p); });
                    }
                    else
                    {
                        tex = std::make_unique<noise_texture>(10.f,
                            [](const perlin& n, const glm::vec3& p) { return 0.5f * (1.f + glm::sin(p.z + 10.f * turbulence(n, p))); });
                    }
                    mat = std::make_unique<lambertian>(std::move(tex));
                }
                else if (choose_material < 0.7f)
                {
                    mat = std::make_unique<metal>(color{ 0.5f } +(0.5f * random_color()), random_uniform(0.f, 0.5f));
                }
                else
                {
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

    w.spawn_object<sphere>(position{ 4.f, 1.f, 0.f }, 1.f,
        std::make_unique<lambertian>(std::make_unique<image_texture>("textures/earthmap.jpg")));
    w.spawn_object<sphere>(position{ 0.f, 1.f, 0.f }, 1.f, std::make_unique<dielectric>(color{ 1.f }, 1.5f));
    w.spawn_object<sphere>(position{ 0.f, 1.f, 0.f }, -0.9f, std::make_unique<dielectric>(color{ 1.f }, 1.5f));
    w.spawn_object<sphere>(position{ -4.f, 1.f, 0.f }, 1.f, std::make_unique<metal>(color{ 0.7f, 0.6f, 0.5f }, 0.f));

    return plan{ image_size, cam, std::move(w) };
}

plan plan::two_noise_spheres(const extent_2d<uint32_t>& image_size)
{
    const camera cam = camera_create_info{
        position{ 13.f, 2.f, 3.f },
        position{ 0.f, 0.f, 0.f },
        axis{ 0.f, 1.f, 0.f },
        20.f,
        float(image_size.width) / float(image_size.height),
        0.05f,
        { 0.f, 1.f }
    };

    scene w;
    w.spawn_object<sphere>(position{ 0.f, -1000.f, 0.f }, 1000.f,
        std::make_unique<lambertian>(std::make_unique<noise_texture>(3.f,
            [](const perlin& n, const glm::vec3& p) { return turbulence(n, p); })));
    w.spawn_object<sphere>(position{ 0.f, 2.f, 0.f }, 2.f,
        std::make_unique<lambertian>(std::make_unique<noise_texture>(2.f,
            [](const perlin& n, const glm::vec3& p) { return 0.5f * (1.f + glm::sin(p.z + 10.f * turbulence(n, p))); })));

    return plan{ image_size, cam, std::move(w) };
}

plan plan::space(const extent_2d<uint32_t>& image_size)
{
    const camera cam = camera_create_info{
        position{ 15.f, 2.f, 15.f },
        position{ 0.f, 0.f, 0.f },
        axis{ 0.f, 1.f, 0.f },
        45.f,
        float(image_size.width) / float(image_size.height),
        0.05f,
        { 0.f, 1.f }
    };

    scene w;
    w.spawn_object<sphere>(position{ 0.f, 0.f, 0.f }, 10000.f,
        std::make_unique<lambertian>(std::make_unique<image_texture>("textures/stars_milky_way.jpg")));
    w.spawn_object<sphere>(position{ 0.f, 0.f, 0.f }, 5.f,
        std::make_unique<diffuse_light>(std::make_unique<image_texture>("textures/sun.jpg")));
    w.spawn_object<sphere>(position{ 8.f, 0.f, -8.f }, 1.f,
        std::make_unique<lambertian>(std::make_unique<image_texture>("textures/earthmap.jpg")));
    return { image_size, cam, std::move(w) };
}