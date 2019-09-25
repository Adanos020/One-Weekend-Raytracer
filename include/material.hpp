#pragma once

#include <ray.hpp>
#include <types.hpp>

#include <memory>
#include <optional>

struct scattering
{
    color attenuation;
    ray scattered_ray;
};

using scattering_opt = std::optional<scattering>;

class material
{
public:
    virtual scattering_opt scatter(const ray&, const struct hit_record&) const = 0;
};

using unique_material = std::unique_ptr<material>;