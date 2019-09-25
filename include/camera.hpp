#pragma once

#include <types.hpp>

struct camera_create_info
{
    position camera_position;
    position looking_at;
    axis up;
    float vertical_fov;
    float aspect_ratio;
    float aperture;
    float focus_distance;
};

class camera
{
public:
    camera(const camera_create_info&);

    struct ray shoot_ray_at(const float u, const float v) const;

private:
    position origin;
    position lower_left_corner;
    position horizontal;
    position vertical;
    displacement w;
    displacement u;
    displacement v;
    float lens_radius;
};