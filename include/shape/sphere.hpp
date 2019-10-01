#pragma once

#include <hittable.hpp>
#include <material.hpp>
#include <util/types.hpp>

class sphere : public hittable
{
public:
    from_to<position> center_transition;
    min_max<float> time_transition;
    float radius;
    unique_material mat;

    sphere() = default;
    sphere(const from_to<position>& center_transition, const min_max<float> time_transition,
        const float radius, unique_material&&);
    sphere(const position& center, const float radius, unique_material&&);

    virtual hit_record_opt hit(const struct ray&, const min_max<float> t) const override;
    virtual axis_aligned_bounding_box_opt bounding_box(const min_max<float> t) const override;
    position center_at_time(const float time) const;

private:
    float inverse_time_interval = 1.f;
};