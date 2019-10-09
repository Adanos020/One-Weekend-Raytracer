#pragma once

#include <line.hpp>
#include <util/types.hpp>

#include <memory>
#include <optional>

struct scattering
{
    color attenuation;
    line scattered_ray;
};

using scattering_opt = std::optional<scattering>;

class material
{
public:
    virtual ~material() = default;
    virtual scattering_opt scatter(const line&, const struct hit_record&) const = 0;
    virtual color emitted(const std::pair<float, float> uv, const position&) const;
};

using unique_material = std::unique_ptr<material>;