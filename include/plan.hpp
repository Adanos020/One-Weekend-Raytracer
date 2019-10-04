#pragma once

#include <camera.hpp>
#include <util/types.hpp>
#include <scene.hpp>

struct plan
{
    extent_2d<uint32_t> image_size;
    camera cam;
    scene w;

    static plan random_balls(const extent_2d<uint32_t>&);
    static plan two_noise_spheres(const extent_2d<uint32_t>&);
    static plan space(const extent_2d<uint32_t>&);
};