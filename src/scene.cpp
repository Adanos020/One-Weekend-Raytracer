#include <scene.hpp>

#include <bounding_volume_hierarchy/bounding_volume_hierarchy_node.hpp>

scene::scene(unique_texture&& sky)
    : sky(std::move(sky))
{
}

hit_record_opt scene::hit(const line& ray, const min_max<float> t) const
{
    static const bounding_volume_hierarchy_node bvh = { this->hittables, t };
    return bvh.hit(ray, t);
}

axis_aligned_bounding_box_opt scene::bounding_box(const min_max<float> t) const
{
    return this->box;
}