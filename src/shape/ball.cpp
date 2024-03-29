#include <shape/ball.hpp>

#include <line.hpp>
#include <math/sphere.hpp>

ball::ball(const position& center, const float radius, unique_material&& mat)
    : ball({ center, center }, { 0.f, 0.f }, radius, std::move(mat))
{
}

ball::ball(const from_to<position>& center_transition, const min_max<float> time_transition,
    const float radius, unique_material&& mat)
    : center_transition(center_transition)
    , time_transition(time_transition)
    , radius(radius)
    , inverse_radius(1.f / radius)
    , mat(std::move(mat))
{
    if (const float interval = time_transition.max - time_transition.min; interval != 0.f)
    {
        this->inverse_time_interval = 1.f / interval;
    }
}

hit_record_opt ball::hit(const line& ray, const min_max<float> t) const
{
    const displacement oc = ray.origin - this->center_at_time(ray.time);
    const float a = glm::dot(ray.direction, ray.direction);
    const float b = glm::dot(oc, ray.direction);
    const float c = glm::dot(oc, oc) - glm::pow(this->radius, 2);
    const float discriminant = b * b - a * c;

    if (discriminant > 0.f)
    {
        if (const float root_1 = (-b - glm::sqrt(discriminant)) / a; root_1 < t.max && root_1 > t.min)
        {
            const position point = ray.point_at_parameter(root_1);
            const std::pair<float, float> uv = this->uv_at(point, ray.time);
            return hit_record{ root_1, point, (point - this->center_at_time(ray.time)) / this->radius, this->mat.get(), uv };
        }
        if (const float root_2 = (-b + glm::sqrt(discriminant)) / a; root_2 < t.max && root_2 > t.min)
        {
            const position point = ray.point_at_parameter(root_2);
            const std::pair<float, float> uv = this->uv_at(point, ray.time);
            return hit_record{ root_2, point, (point - this->center_at_time(ray.time)) / this->radius, this->mat.get(), uv };
        }
    }
    return {};
}

axis_aligned_bounding_box_opt ball::bounding_box(const min_max<float> t) const
{
    const float r = glm::abs(this->radius);
    return axis_aligned_bounding_box::surrounding(
        axis_aligned_bounding_box{
            position{ this->center_transition.from - displacement{ r } },
            position{ this->center_transition.from + displacement{ r } } },
        axis_aligned_bounding_box{
            position{ this->center_transition.to - displacement{ r } },
            position{ this->center_transition.to + displacement{ r } } });
}

position ball::center_at_time(const float time) const
{
    const position from = this->center_transition.from;
    const position to = this->center_transition.to;
    const float t_min = this->time_transition.min;
    return from + ((time - t_min) * this->inverse_time_interval) * (to - from);
}

std::pair<float, float> ball::uv_at(const position& p, const float time) const
{
    return uv_on_sphere(glm::abs(this->inverse_radius) * (p - this->center_at_time(time)));
}