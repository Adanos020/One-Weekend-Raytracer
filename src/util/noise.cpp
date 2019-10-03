#include <util/noise.hpp>

#include <util/interpolation.hpp>
#include <util/random.hpp>

#include <algorithm>
#include <numeric>

perlin::perlin()
{
    std::generate(this->random_vectors.begin(), this->random_vectors.end(), [] {
        return glm::normalize(glm::vec3{
            random_uniform(-1.f, 1.f), random_uniform(-1.f, 1.f), random_uniform(-1.f, 1.f)
        });
    });

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
    const int i = glm::floor(p.x);
    const int j = glm::floor(p.y);
    const int k = glm::floor(p.z);

    std::array<glm::vec3, 8> c;
    for (size_t it = 0; it < c.size(); ++it)
    {
        const size_t di = (it & 4) >> 2;
        const size_t dj = (it & 2) >> 1;
        const size_t dk = (it & 1) >> 0;
        c[it] = this->random_vectors[
            this->x_permutations[(i + di) & 255] ^
            this->y_permutations[(j + dj) & 255] ^
            this->z_permutations[(k + dk) & 255]];
    }
    return perlin_trilerp(c, u, v, w);
}