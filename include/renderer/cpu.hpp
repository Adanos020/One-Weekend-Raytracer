#pragma once

#include <render_plan.hpp>
#include <util/types.hpp>

#include <future>
#include <vector>

class cpu_renderer
{
public:
    cpu_renderer(const uint32_t sample_count, const uint32_t thread_count);
    std::vector<rgb> render_scene(const render_plan&);

private:
    std::vector<rgb> render_fragment(const render_plan*, const glm::uvec2 top_left, const glm::uvec2 bottom_right);

private:
    const uint32_t sample_count;
    const uint32_t thread_count;

    float progress = 0.f;
    mutable std::mutex progress_mtx;
};