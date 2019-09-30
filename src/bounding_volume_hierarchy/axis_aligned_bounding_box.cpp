#include <bounding_volume_hierarchy/axis_aligned_bounding_box.hpp>

#include <ray.hpp>

#include <algorithm>

axis_aligned_bounding_box axis_aligned_bounding_box::surrounding(
    const axis_aligned_bounding_box& b1, const axis_aligned_bounding_box& b2)
{
    const position top_left_back = {
        std::fmax(b1.top_left_back.x, b2.top_left_back.x),
        std::fmax(b1.top_left_back.y, b2.top_left_back.y),
        std::fmax(b1.top_left_back.z, b2.top_left_back.z)
    };
    const position bottom_right_front = {
        std::fmin(b1.top_left_back.x, b2.top_left_back.x),
        std::fmin(b1.top_left_back.y, b2.top_left_back.y),
        std::fmin(b1.top_left_back.z, b2.top_left_back.z)
    };
    return axis_aligned_bounding_box{ top_left_back, bottom_right_front };
}

bool axis_aligned_bounding_box::hit(const ray& r, min_max<float> t) const
{
#define check_coord(coord)\
    {\
        const min_max<float> temp = std::minmax(\
            (this->top_left_back.coord - r.origin.coord) / r.direction.coord,\
            (this->bottom_right_front.coord - r.origin.coord) / r.direction.coord);\
        t.min = std::min(temp.min, t.min);\
        t.max = std::max(temp.max, t.max);\
        if (t.max <= t.min)\
        {\
            return false;\
        }\
    }

    check_coord(x);
    check_coord(y);
    check_coord(z);
#undef calc
    return true;
}