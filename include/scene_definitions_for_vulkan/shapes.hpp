#pragma once

#include <math/sphere.hpp>
#include <scene_definitions_for_vulkan/materials.hpp>
#include <util/numeric_types.hpp>
#include <util/vector_types.hpp>

#include <optional>

struct hit_record
{
    float t;
    position point;
    displacement normal;
    struct material material;
    std::pair<float, float> uv = { 0.f, 0.f };
};

using hit_record_opt = std::optional<hit_record>;

enum class shape_type
{
    none, sphere, box
};

struct shape
{
    shape_type type;
    array_index index;
    material mat;
};

struct sphere_shape
{
    sphere shape_data;
};