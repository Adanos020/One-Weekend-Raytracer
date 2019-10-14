#pragma once

#include <util/pairs.hpp>
#include <util/vector_types.hpp>

struct camera_create_info
{
    position camera_position;
    position looking_at;
    axis up;
    float vertical_fov;
    float aspect_ratio;
    float aperture;
    min_max<float> time;
};

class camera
{
public:
    position origin;

public:
    camera(const camera_create_info&);

    struct line shoot_ray_at(const float u, const float v) const;

private:
    position lower_left_corner;
    position horizontal;
    position vertical;
    displacement w;
    displacement u;
    displacement v;
    float lens_radius;
    min_max<float> time;
};