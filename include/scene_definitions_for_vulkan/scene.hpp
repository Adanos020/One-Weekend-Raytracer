#pragma once

#include <scene_definitions_for_vulkan/materials.hpp>
#include <scene_definitions_for_vulkan/shapes.hpp>
#include <scene_definitions_for_vulkan/textures.hpp>

#include <vector>

struct scene
{
    texture sky;

    std::vector<shape> shapes;
    std::vector<sphere_shape> sphere_shapes;

    std::vector<dielectric_material> dielectric_materials;
    std::vector<diffuse_light_material> diffuse_light_materials;
    std::vector<lambertian_material> lambertian_materials;
    std::vector<metal_material> metal_materials;

    std::vector<checker_texture> checker_textures;
    std::vector<constant_texture> constant_textures;
    std::vector<image_texture> image_textures;
    std::vector<noise_texture> noise_textures;

    shape add_shape(const sphere_shape&);

    material add_material(const dielectric_material&);
    material add_material(const diffuse_light_material&);
    material add_material(const lambertian_material&);
    material add_material(const metal_material&);

    texture add_texture(const checker_texture&);
    texture add_texture(const constant_texture&);
    texture add_texture(const image_texture&);
    texture add_texture(const noise_texture&);

    std::vector<uint8_t> to_bytes() const;
};