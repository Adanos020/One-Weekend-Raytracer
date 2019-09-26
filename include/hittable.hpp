#pragma once

#include <types.hpp>

#include <memory>
#include <optional>

struct hit_record
{
    float t;
    position point;
    displacement normal;
    class material* p_material;
};

using hit_record_opt = std::optional<hit_record>;

class hittable
{
public:
    ~hittable() = default;
    virtual hit_record_opt hit(const struct ray&, const float t_min, const float t_max) const = 0;
};

using unique_hittable = std::unique_ptr<hittable>;