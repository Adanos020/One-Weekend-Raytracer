#pragma once

#include <hittable.hpp>
#include <material.hpp>
#include <util/types.hpp>

class sphere : public hittable
{
public:
    sphere() = default;
    sphere(const from_to<position>& center_transition, const min_max<float> time_transition,
        const float radius, unique_material&&);
    sphere(const position& center, const float radius, unique_material&&);

    virtual hit_record_opt hit(const struct ray&, const min_max<float> t) const override;
    virtual axis_aligned_bounding_box_opt bounding_box(const min_max<float> t) const override;
    position center_at_time(const float time) const;
    std::pair<float, float> uv_at(const position&, const float time) const;

private:
    from_to<position> center_transition;
    min_max<float> time_transition;
    float radius;
    float inverse_radius;
    unique_material mat;
    float inverse_time_interval = 1.f;
};