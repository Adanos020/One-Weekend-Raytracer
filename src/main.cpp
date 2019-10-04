#include <ray.hpp>
#include <plan.hpp>
#include <util/random.hpp>
#include <util/types.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <array>
#include <fstream>
#include <future>
#include <iostream>
#include <iomanip>

static float progress = 0.f;
static std::mutex stdout_mtx;

std::vector<rgb> render_fragment(const plan* in_plan, const glm::uvec2 top_left, const glm::uvec2 bottom_right)
{
    const uint32_t sample_count = 500;
    const uint32_t width = bottom_right.x - top_left.x;
    const uint32_t height = bottom_right.y - top_left.y;
    const float inverse_image_width = 1.f / in_plan->image_size.width;
    const float inverse_image_height = 1.f / in_plan->image_size.height;
    const float inverse_sample_count = 1.f / sample_count;

    std::vector<rgb> image_fragment;
    image_fragment.reserve(size_t(width) * height);

    for (uint32_t y = top_left.y; y < bottom_right.y; ++y)
    {
        for (uint32_t x = top_left.x; x < bottom_right.x; ++x)
        {
            color col{ 0.f };
            for (uint32_t s = 0; s < sample_count; ++s)
            {
                const float u = float(x + random_uniform(0.f, 1.f)) * inverse_image_width;
                const float v = float(in_plan->image_size.height - y + random_uniform(0.f, 1.f)) * inverse_image_height;
                const ray r = in_plan->cam.shoot_ray_at(u, v);
                col += r.seen_color(in_plan->w, 0);
            }
            col *= inverse_sample_count;
            col = { glm::sqrt(col.r), glm::sqrt(col.g), glm::sqrt(col.b) };
            col *= 255.99f;

            image_fragment.push_back({ uint8_t(col.r), uint8_t(col.g), uint8_t(col.b) });
        }

        std::lock_guard lock{ stdout_mtx };
        progress += 100.f * inverse_image_height;
        std::cout << "\rRendering image fragments... " << std::fixed << std::setprecision(2) << progress << "%";
    }
    return image_fragment;
}

std::vector<rgb> render_scene(const plan& in_plan)
{
    const uint32_t render_thread_count = 20;
    const uint32_t fragment_height = in_plan.image_size.height / render_thread_count;

    std::cout << "Starting jobs... ";

    std::array<std::future<std::vector<rgb>>, render_thread_count> image_fragments;
    for (uint32_t i = 0; i < image_fragments.size(); ++i)
    {
        image_fragments[i] = std::async(std::launch::async, render_fragment,
            &in_plan, glm::uvec2{ 0, i * fragment_height },
            glm::uvec2{ in_plan.image_size.width, (i + 1) * fragment_height });
    }

    std::cout << "Done." << std::endl;
    std::cout << "Rendering image fragments... 0.00%";

    std::vector<rgb> image;
    for (uint32_t i = 0; i < image_fragments.size(); ++i)
    {
        const std::vector<rgb> fragment = image_fragments[i].get();
        image.insert(image.end(), fragment.begin(), fragment.end());
    }

    std::cout << "\rRendering image fragments... Done.  " << std::endl;
    return image;
}

void export_image(const std::vector<rgb>& image, const extent_2d<uint32_t> image_size, const std::string_view path)
{
    std::cout << "Writing to file... ";

    const uint32_t channels = 3;
    stbi_write_png(path.data(), image_size.width, image_size.height, channels, image.data(), image_size.width * channels);

    std::cout << "Done." << std::endl;
}

int main()
{
    const extent_2d<uint32_t> image_size = { 1600, 900 };
    const plan render_plan = plan::random_balls(image_size);
    const std::vector<rgb> image = render_scene(render_plan);
    export_image(image, image_size, "image.png");
    return EXIT_SUCCESS;
}