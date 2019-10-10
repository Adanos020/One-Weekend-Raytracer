#include <renderer/cpu.hpp>

#include <line.hpp>
#include <util/random.hpp>

#include <iomanip>
#include <iostream>

cpu_renderer::cpu_renderer(const uint32_t sample_count, const uint32_t thread_count)
    : sample_count(sample_count)
    , thread_count(thread_count)
{
}

std::vector<rgb> cpu_renderer::render_scene(const render_plan& plan)
{
    const uint32_t fragment_height = plan.image_size.height / this->thread_count;

    std::cout << "Starting jobs... ";

    std::vector<std::future<std::vector<rgb>>> image_fragments{ this->thread_count };
    for (uint32_t i = 0; i < image_fragments.size(); ++i)
    {
        image_fragments[i] = std::async(std::launch::async, &cpu_renderer::render_fragment, this,
            &plan, glm::uvec2{ 0, i * fragment_height },
            glm::uvec2{ plan.image_size.width, (i + 1) * fragment_height });
    }

    std::cout << "Done." << std::endl;
    std::cout << "Rendering image fragments... 0.00%";

    this->progress = 0.f;
    std::vector<rgb> image;
    for (uint32_t i = 0; i < image_fragments.size(); ++i)
    {
        const std::vector<rgb> fragment = image_fragments[i].get();
        image.insert(image.end(), fragment.begin(), fragment.end());
    }

    std::cout << "\rRendering image fragments... Done.  " << std::endl;
    return image;
}

std::vector<rgb> cpu_renderer::render_fragment(const render_plan* plan, const glm::uvec2 top_left, const glm::uvec2 bottom_right)
{
    const uint32_t width = bottom_right.x - top_left.x;
    const uint32_t height = bottom_right.y - top_left.y;
    const float inverse_image_width = 1.f / plan->image_size.width;
    const float inverse_image_height = 1.f / plan->image_size.height;
    const float inverse_sample_count = 1.f / sample_count;

    std::vector<rgb> image_fragment;
    image_fragment.reserve(size_t(width) * size_t(height));

    for (uint32_t y = top_left.y; y < bottom_right.y; ++y)
    {
        for (uint32_t x = top_left.x; x < bottom_right.x; ++x)
        {
            color col{ 0.f };
            for (uint32_t s = 0; s < sample_count; ++s)
            {
                const float u = float(x + random_uniform(0.f, 1.f)) * inverse_image_width;
                const float v = float(plan->image_size.height - y + random_uniform(0.f, 1.f)) * inverse_image_height;
                const line ray = plan->cam.shoot_ray_at(u, v);
                col += ray.seen_color(plan->world);
            }
            col *= inverse_sample_count;
            col = { glm::sqrt(col.r), glm::sqrt(col.g), glm::sqrt(col.b) };

            image_fragment.push_back(rgb{ col * 255.99f });
        }

        std::lock_guard lock{ this->progress_mtx };
        this->progress += 100.f * inverse_image_height;
        std::cout << "\rRendering image fragments... " << std::fixed << std::setprecision(2) << this->progress << "%";
    }
    return image_fragment;
}