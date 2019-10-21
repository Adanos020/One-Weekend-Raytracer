#pragma once

#include <camera.hpp>
#include <scene_definitions_for_vulkan/scene.hpp>
#include <util/sizes.hpp>

struct render_plan
{
    extent_2d<uint32_t> image_size;
    camera cam;
    scene world;

    static render_plan hello_ball(const extent_2d<uint32_t>& image_size);
};