#include <util/noise.hpp>

#include <util/random.hpp>

#include <algorithm>
#include <numeric>

perlin::perlin()
{
    std::generate(this->random_numbers.begin(), this->random_numbers.end(), []{ return random_uniform<float>(); });

    std::iota(this->x_permutations.begin(), this->x_permutations.end(), 0);
    std::iota(this->y_permutations.begin(), this->y_permutations.end(), 0);
    std::iota(this->z_permutations.begin(), this->z_permutations.end(), 0);

    std::default_random_engine rng;
    std::shuffle(this->x_permutations.begin(), this->x_permutations.end(), rng);
    std::shuffle(this->y_permutations.begin(), this->y_permutations.end(), rng);
    std::shuffle(this->z_permutations.begin(), this->z_permutations.end(), rng);
}

float perlin::noise(const glm::vec3& p) const
{
    const float u = p.x - glm::floor(p.x);
    const float v = p.y - glm::floor(p.y);
    const float w = p.z - glm::floor(p.z);
    const int i = int(4 * p.x) & 255;
    const int j = int(4 * p.y) & 255;
    const int k = int(4 * p.z) & 255;
    return this->random_numbers[this->x_permutations[i] ^ this->y_permutations[j] ^ this->z_permutations[k]];
}