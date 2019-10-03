#pragma once

#include <ray.hpp>
#include <util/types.hpp>

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
    virtual ~material() = default;
    virtual scattering_opt scatter(const ray&, const struct hit_record&) const = 0;

    class sphere* parent_shape;
};

using unique_material = std::unique_ptr<material>;