#pragma once

#include <texture.hpp>
#include <util/noise.hpp>

#include <functional>

using noise_transform_fn = std::function<float(const perlin&, const glm::vec3&)>;

class noise_texture : public texture
{
public:
    noise_texture() = default;
    noise_texture(const float scale, const noise_transform_fn& =
        [](const perlin& n, const glm::vec3& p) { return 0.5f * (1.f + n.noise(p)); });

    virtual color value_at(const float u, const float v, const position&) const override;

private:
    perlin noise;
    float scale;

    noise_transform_fn noise_transform;
};