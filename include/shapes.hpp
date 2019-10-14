#pragma once

#include <materials.hpp>
#include <math/sphere.hpp>
#include <util/numeric_types.hpp>
#include <util/vector_types.hpp>

enum class shape_type
{
    none, sphere, box
};

struct shape
{
    shape_type type;
    array_index index;
};

struct sphere_shape
{
    sphere shape_data;
    material mat;
};