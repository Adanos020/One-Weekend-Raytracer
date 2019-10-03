#pragma once

#include <camera.hpp>
#include <util/types.hpp>
#include <world.hpp>

struct scene
{
    extent_2d<uint32_t> image_size;
    camera cam;
    world w;

    static scene random_world_balls(const extent_2d<uint32_t>&);
    static scene two_noise_spheres(const extent_2d<uint32_t>&);
};