#pragma once

#include <bounding_volume_hierarchy/axis_aligned_bounding_box.hpp>
#include <hittable.hpp>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

class world : public hittable
{
public:
    static world random_world_balls();

    template<class T, typename... Args>
    void spawn_object(Args... args)
    {
        static_assert(std::is_base_of_v<hittable, T>);
        this->hittables.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        if (const axis_aligned_bounding_box_opt last_box =
            this->hittables.back()->bounding_box(min_max<float>{ 0.0001f, FLT_MAX }))
        {
            this->box = axis_aligned_bounding_box::surrounding(this->box, *last_box);
        }
    }

    virtual hit_record_opt hit(const struct ray&, const min_max<float> t) const override;
    virtual axis_aligned_bounding_box_opt bounding_box(const min_max<float> t) const override;

private:
    mutable std::vector<unique_hittable> hittables;
    axis_aligned_bounding_box box;
};