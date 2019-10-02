#pragma once

#include <glm/glm.hpp>

#include <array>

class perlin
{
public:
    perlin();

    float noise(const glm::vec3&) const;

private:
    inline static constexpr size_t N_PERMUTATIONS = 256;
    std::array<float, N_PERMUTATIONS> random_numbers;
    std::array<int32_t, N_PERMUTATIONS> x_permutations;
    std::array<int32_t, N_PERMUTATIONS> y_permutations;
    std::array<int32_t, N_PERMUTATIONS> z_permutations;
};