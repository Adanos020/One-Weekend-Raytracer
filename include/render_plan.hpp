#pragma once

#include <camera.hpp>
#include <util/types.hpp>
#include <scene.hpp>

struct render_plan
{
    extent_2d<uint32_t> image_size;
    camera cam;
    scene world;

    static render_plan random_balls(const extent_2d<uint32_t>&);
    static render_plan two_noise_spheres(const extent_2d<uint32_t>&);
    static render_plan space(const extent_2d<uint32_t>&);
};