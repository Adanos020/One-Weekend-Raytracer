#pragma once

#include <texture.hpp>
#include <util/noise.hpp>

#include <functional>

using noise_transform_fn = std::function<float(const perlin&, const glm::vec3&)>;

class noise_texture : public texture
{
public:
    noise_texture() = default;
    noise_texture(const float scale, const color& albedo = color{ 1.f }, const noise_transform_fn& =
        [](const perlin& n, const glm::vec3& p) { return 0.5f * (1.f + n.noise(p)); });

    virtual color value_at(const std::pair<float, float> uv, const position&) const override;

private:
    perlin noise;

    float scale;
    color albedo;
    noise_transform_fn noise_transform;
};