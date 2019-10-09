#pragma once

#include <bounding_volume_hierarchy/axis_aligned_bounding_box.hpp>
#include <hittable.hpp>
#include <util/types.hpp>

#include <vector>

class bounding_volume_hierarchy_node : public hittable
{
public:
    bounding_volume_hierarchy_node(const iterator_pair<std::vector<unique_hittable>> hittables,
        const min_max<float> time);
    ~bounding_volume_hierarchy_node();

    virtual hit_record_opt hit(const struct line&, const min_max<float> t) const override;
    virtual axis_aligned_bounding_box_opt bounding_box(const min_max<float> t) const override;

private:
    hittable* left;
    hittable* right;

    bool both_are_bvhs = false;

    axis_aligned_bounding_box box;
};