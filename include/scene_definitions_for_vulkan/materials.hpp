#pragma once

#include <scene_definitions_for_vulkan/textures.hpp>
#include <util/numeric_types.hpp>

enum class material_type
{
    none, dielectric, diffuse_light, lambertian, metal
};

struct material
{
    material_type type;
    array_index index;
};

struct dielectric_material
{
    float refractive_index;
    texture albedo;
};

struct diffuse_light_material
{
    texture emit;
};

struct lambertian_material
{
    texture albedo;
};

struct metal_material
{
    float fuzz;
    texture albedo;
};