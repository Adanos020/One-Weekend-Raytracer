#include <bounding_volume_hierarchy/axis_aligned_bounding_box.hpp>

#include <line.hpp>

#include <algorithm>

axis_aligned_bounding_box axis_aligned_bounding_box::surrounding(
    const axis_aligned_bounding_box& b1, const axis_aligned_bounding_box& b2)
{
    const position top_left_back = {
        std::fmin(b1.min.x, b2.min.x),
        std::fmin(b1.min.y, b2.min.y),
        std::fmin(b1.min.z, b2.min.z) };
    const position bottom_right_front = {
        std::fmax(b1.max.x, b2.max.x),
        std::fmax(b1.max.y, b2.max.y),
        std::fmax(b1.max.z, b2.max.z) };
    return axis_aligned_bounding_box{ top_left_back, bottom_right_front };
}

bool axis_aligned_bounding_box::hit(const line& ray, min_max<float> time) const
{
    const min_max<float> tx = {
        (this->min.x - ray.origin.x) * ray.inverse_direction.x,
        (this->max.x - ray.origin.x) * ray.inverse_direction.x };
    min_max<float> t = std::minmax(tx.min, tx.max);

    const min_max<float> ty = {
        (this->min.y - ray.origin.y) * ray.inverse_direction.y,
        (this->max.y - ray.origin.y) * ray.inverse_direction.y };
    t = {
        std::max(t.min, std::min(ty.min, ty.max)),
        std::min(t.max, std::max(ty.min, ty.max)) };

    const min_max<float> tz = {
        (this->min.z - ray.origin.z) * ray.inverse_direction.z,
        (this->max.z - ray.origin.z) * ray.inverse_direction.z };
    t = {
        std::max(t.min, std::min(tz.min, tz.max)),
        std::min(t.max, std::max(tz.min, tz.max)) };

    return t.max >= t.min;
}