#include <world.hpp>

#include <bounding_volume_hierarchy/bounding_volume_hierarchy_node.hpp>

hit_record_opt world::hit(const ray& r, const min_max<float> t) const
{
    static const bounding_volume_hierarchy_node bvh = { this->hittables, t };
    return bvh.hit(r, t);
}

axis_aligned_bounding_box_opt world::bounding_box(const min_max<float> t) const
{
    return this->box;
}