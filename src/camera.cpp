#include <camera.hpp>

#include <util/random.hpp>
#include <ray.hpp>

camera::camera(const camera_create_info& info)
    : origin(info.camera_position)
    , lens_radius(info.aperture * 0.5f)
{
    const float fov_radians = glm::radians(info.vertical_fov);
    const float half_height = glm::tan(fov_radians * 0.5f);
    const float half_width = info.aspect_ratio * half_height;

    this->w = glm::normalize(info.camera_position - info.looking_at);
    this->u = glm::normalize(glm::cross(info.up, this->w));
    this->v = glm::cross(this->w, this->u);

    this->lower_left_corner = this->origin - (half_width * info.focus_distance * this->u) -
        (half_height * info.focus_distance * this->v) - (info.focus_distance * this->w);
    this->horizontal = 2.f * half_width * info.focus_distance * u;
    this->vertical = 2.f * half_height * info.focus_distance * v;
}

ray camera::shoot_ray_at(const float s, const float t) const
{
    const displacement random_spot_on_lens = this->lens_radius * random_in_unit_disk();
    const displacement offset = (this->u * random_spot_on_lens.x) + (this->v * random_spot_on_lens.y);
    return ray{
        this->origin + offset,
        this->lower_left_corner + (s * this->horizontal) + (t * this->vertical) - origin - offset
    };
}