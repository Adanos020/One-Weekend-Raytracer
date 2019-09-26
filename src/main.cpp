#include <camera.hpp>
#include <material/dielectric.hpp>
#include <material/lambertian.hpp>
#include <material/metal.hpp>
#include <random.hpp>
#include <ray.hpp>
#include <sphere.hpp>
#include <types.hpp>
#include <world.hpp>

#include <fstream>
#include <future>
#include <glm/glm.hpp>
#include <iostream>
#include <iomanip>

static constexpr uint32_t IMAGE_WIDTH = 1600;
static constexpr uint32_t IMAGE_HEIGHT = 900;
static constexpr float FOV = 45.f;
static constexpr uint32_t SAMPLE_COUNT = 500;
static constexpr uint32_t RENDER_THREAD_COUNT = 20;

static float progress = 0.f;
static std::mutex stdout_mtx;

std::vector<color> render_fragment(const world* w, const camera* cam,
    const glm::uvec2 top_left, const glm::uvec2 bottom_right)
{
    const uint32_t width = bottom_right.x - top_left.x;
    const uint32_t height = bottom_right.y - top_left.y;

    std::vector<color> image_fragment;
    image_fragment.reserve(size_t(width) * height);

    for (uint32_t y = top_left.y; y < bottom_right.y; ++y)
    {
        for (uint32_t x = top_left.x; x < bottom_right.x; ++x)
        {
            color col{ 0.f };
            for (uint32_t s = 0; s < SAMPLE_COUNT; ++s)
            {
                const float u = float(x + random_uniform(0.f, 1.f)) / IMAGE_WIDTH;
                const float v = float(IMAGE_HEIGHT - y + random_uniform(0.f, 1.f)) / IMAGE_HEIGHT;
                const ray r = cam->shoot_ray_at(u, v);
                col += r.seen_color(*w, 0);
            }
            col /= float(SAMPLE_COUNT);
            col = { glm::sqrt(col.r), glm::sqrt(col.g), glm::sqrt(col.b) };
            col *= 255.99f;

            image_fragment.push_back(col);
        }

        std::lock_guard lock{ stdout_mtx };
        progress += 100.f / IMAGE_HEIGHT;
        std::cout << "\rRendering image fragments... " << std::fixed << std::setprecision(2) << progress << "%";
    }
    return image_fragment;
}

std::vector<color> render_image(const world& w, const camera& cam)
{
    const uint32_t fragment_height = IMAGE_HEIGHT / RENDER_THREAD_COUNT;

    std::cout << "Starting jobs... ";

    std::vector<std::future<std::vector<color>>> image_fragments{ RENDER_THREAD_COUNT };
    for (uint32_t i = 0; i < image_fragments.size(); ++i)
    {
        image_fragments[i] = std::async(std::launch::async, render_fragment,
            &w, &cam, glm::uvec2{ 0, i * fragment_height }, glm::uvec2{ IMAGE_WIDTH, (i + 1) * fragment_height });
    }

    std::cout << "Done." << std::endl;
    std::cout << "Rendering image fragments... 0.00%";

    std::vector<color> image;
    for (uint32_t i = 0; i < image_fragments.size(); ++i)
    {
        const std::vector<color> fragment = image_fragments[i].get();
        image.insert(image.end(), fragment.begin(), fragment.end());
    }

    std::cout << "\rRendering image fragments... Done.  " << std::endl;
    return image;
}

int main()
{
    const position camera_position = { 8.f, 3.f, 3.f };
    const position look_at = { 0.f, 0.f, -1.f };
    const camera cam = camera_create_info{
        camera_position,
        look_at,
        axis{ 0.f, 1.f, 0.f },
        FOV,
        float(IMAGE_WIDTH) / float(IMAGE_HEIGHT),
        0.1f,
        glm::distance(camera_position, look_at)
    };

    const world w = world::random_world_balls();
    //     w.spawn_object<sphere>(position{ 0.f, 0.25f, -0.75f }, 0.75f, std::make_unique<lambertian>(color{ 0.8f, 0.3f, 0.3f }));
    //     w.spawn_object<sphere>(position{ 0.f, -50.5f, 0.f }, 50.f, std::make_unique<lambertian>(color{ 0.8f, 0.8f, 0.f }));
    //     w.spawn_object<sphere>(position{ 0.f, -0.25f, 0.5f }, 0.25f, std::make_unique<dielectric>(color{ 1.f, 1.f, 1.f }, 1.5f));
    //     w.spawn_object<sphere>(position{ -0.75f, 0.f, 0.25f }, 0.5f, std::make_unique<dielectric>(color{ 1.f, 0.6f, 0.6f }, 1.5f));
    //     w.spawn_object<sphere>(position{ -0.75f, 0.f, 0.25f }, -0.45f, std::make_unique<dielectric>(color{ 1.f, 1.f, 1.f }, 1.5f));
    //     w.spawn_object<sphere>(position{ 0.75f, 0.f, 0.25f }, 0.5f, std::make_unique<metal>(color{ 0.8f, 0.6f, 0.2f }, 0.05f));

    const std::vector<color> image = render_image(w, cam);

    std::cout << "Writing to file... ";

    std::ofstream file{ "image.ppm" };
    file << "P3\n" << IMAGE_WIDTH << " " << IMAGE_HEIGHT << "\n255\n";
    for (const color& col : image)
    {
        file << uint32_t(col.r) << " " << uint32_t(col.g) << " " << uint32_t(col.b) << "\n";
    }

    std::cout << "Done." << std::endl;

    return EXIT_SUCCESS;
}