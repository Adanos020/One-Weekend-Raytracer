#include <render_plan.hpp>

#include <material/diffuse_light.hpp>
#include <material/dielectric.hpp>
#include <material/lambertian.hpp>
#include <material/metal.hpp>
#include <shape/ball.hpp>
#include <texture/checker.hpp>
#include <texture/constant.hpp>
#include <texture/image.hpp>
#include <texture/noise.hpp>
#include <util/noise.hpp>
#include <util/random.hpp>

render_plan render_plan::random_balls(const extent_2d<uint32_t>& image_size)
{
    const camera cam = camera_create_info{
        position{ 4.f, 3.f, 6.f },
        position{ 0.f, 1.f, 0.f },
        y_axis,
        45.f,
        image_size.aspect(),
        0.05f,
        { 0.f, 1.f }
    };

    scene world{ std::make_unique<image_texture>("textures/stars_milky_way.jpg") };
    world.spawn_object<ball>(position{ 0.f, -1000.f, 0.f }, 1000.f, std::make_unique<lambertian>(
        std::make_unique<noise_texture>(20.f, color{ 1.f, 1.f, 0.7f },
        [](const perlin& n, const glm::vec3& p) { return 0.5f * (1 + turbulence(n, p)); })));

    for (int32_t a = -11; a < 11; ++a)
    {
        for (int32_t b = -11; b < 11; ++b)
        {
            if (const position center = position{ float(a), 0.2f, float(b) } + (0.3f * random_in_unit_disk(y_axis));
                glm::distance(center, position{ 4.f, 0.2f, 0.f }) > 0.9f)
            {
                unique_material mat;
                if (const float choose_material = random_uniform<float>(); choose_material < 0.5f)
                {
                    unique_texture tex;
                    if (const float choose_texture = random_uniform<float>(); choose_texture < 0.25f)
                    {
                        tex = std::make_unique<checker_texture>(30.f, random_color(), random_color());
                    }
                    else if (choose_texture < 0.5f)
                    {
                        tex = std::make_unique<constant_texture>(random_color());
                    }
                    else if (choose_texture < 0.75f)
                    {
                        tex = std::make_unique<noise_texture>(10.f, random_color(),
                            [](const perlin& n, const glm::vec3& p) { return turbulence(n, p); });
                    }
                    else
                    {
                        tex = std::make_unique<noise_texture>(10.f, random_color(),
                            [](const perlin& n, const glm::vec3& p) { return 0.5f * (1.f + glm::sin(p.z + 10.f * turbulence(n, p))); });
                    }
                    mat = std::make_unique<lambertian>(std::move(tex));
                }
                else if (choose_material < 0.65f)
                {
                    mat = std::make_unique<metal>(color{ 0.5f } + (0.5f * random_color()), random_uniform(0.f, 0.5f));
                }
                else if (choose_material < 0.8f)
                {
                    mat = std::make_unique<dielectric>(color{ 0.5f } + (0.5f * random_color()), random_uniform(1.5f, 2.5f));
                }
                else
                {
                    const float index = random_uniform(1.5f, 2.5f);
                    mat = std::make_unique<dielectric>(color{ 0.5f } + (0.5f * random_color()), index);
                    world.spawn_object<ball>(center, -0.18f, std::make_unique<dielectric>(color{ 1.f }, index));
                }
                world.spawn_object<ball>(center, 0.2f, std::move(mat));
            }
        }
    }

    world.spawn_object<ball>(position{ 0.f, 1.f, -4.f }, 1.f,
        std::make_unique<lambertian>(std::make_unique<image_texture>("textures/earth.jpg")));
    
    world.spawn_object<ball>(position{ 0.f, 1.f, 0.f }, 1.f, std::make_unique<dielectric>(color{ 1.f }, 1.5f));
    world.spawn_object<ball>(position{ 0.f, 1.f, 0.f }, -0.9f, std::make_unique<dielectric>(color{ 1.f }, 1.5f));

    world.spawn_object<ball>(position{ -4.f, 1.f, 0.f }, 1.f, std::make_unique<metal>(
        std::make_unique<noise_texture>(3.f, color{ 0.7f, 0.6f, 0.5f }), 0.f));

    world.spawn_object<ball>(position{ 0.f, 4.f, -2.f }, 1.f, std::make_unique<diffuse_light>(color{ 1.f }));
    world.spawn_object<ball>(position{ -2.f, 3.f, -2.f }, 1.f, std::make_unique<diffuse_light>(color{ 1.f }));
    world.spawn_object<ball>(position{ -2.f, 4.f, 0.f }, 1.f, std::make_unique<diffuse_light>(color{ 1.f }));

    return render_plan{ image_size, cam, std::move(world) };
}

render_plan render_plan::two_noise_spheres(const extent_2d<uint32_t>& image_size)
{
    const camera cam = camera_create_info{
        position{ 13.f, 2.f, 3.f },
        position{ 0.f, 0.f, 0.f },
        y_axis,
        20.f,
        image_size.aspect(),
        0.05f,
        { 0.f, 1.f }
    };

    scene world;
    world.spawn_object<ball>(position{ 0.f, -1000.f, 0.f }, 1000.f,
        std::make_unique<lambertian>(std::make_unique<noise_texture>(3.f, color{ 1.f },
            [](const perlin& n, const glm::vec3& p) { return turbulence(n, p); })));
    world.spawn_object<ball>(position{ 0.f, 2.f, 0.f }, 2.f,
        std::make_unique<lambertian>(std::make_unique<noise_texture>(2.f, color{ 1.f },
            [](const perlin& n, const glm::vec3& p) { return 0.5f * (1.f + glm::sin(p.z + 10.f * turbulence(n, p))); })));

    return render_plan{ image_size, cam, std::move(world) };
}

render_plan render_plan::space(const extent_2d<uint32_t>& image_size)
{
    const camera cam = camera_create_info{
        position{ 15.f, 2.f, 15.f },
        position{ 0.f, 0.f, 0.f },
        y_axis,
        45.f,
        image_size.aspect(),
        0.05f,
        { 0.f, 1.f }
    };

    scene world{ std::make_unique<image_texture>("textures/stars_milky_way.jpg") };
    world.spawn_object<ball>(position{ 0.f, 0.f, 0.f }, 5.f,
        std::make_unique<diffuse_light>(std::make_unique<image_texture>("textures/sun.jpg")));
    world.spawn_object<ball>(position{ 8.f, 0.f, -8.f }, 1.f,
        std::make_unique<lambertian>(std::make_unique<image_texture>("textures/earth.jpg")));
    world.spawn_object<ball>(position{ 8.f, 0.f, -8.f }, 1.025f,
        std::make_unique<dielectric>(std::make_unique<image_texture>("textures/earth_clouds.png"), 1.000293f));

    return render_plan{ image_size, cam, std::move(world) };
}