#pragma once

#include <util/colors.hpp>
#include <util/numeric_types.hpp>
#include <util/sizes.hpp>

enum class texture_type
{
    none, checker, constant, image, noise
};

struct texture
{
    texture_type type;
    array_index index;
};

struct checker_texture
{
    float scale;
    texture odd;
    texture even;
};

struct constant_texture
{
    color_alpha value;
};

struct image_texture
{
    array_index image_index;
    extent_2d<uint32_t> size;
};

struct noise_texture
{
    float scale;
    color_alpha base_color;
};