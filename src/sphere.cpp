#include <sphere.hpp>

#include <ray.hpp>

sphere::sphere(const position& center, const float radius, unique_material&& mat)
    : center(center)
    , radius(radius)
    , mat(std::move(mat))
{
}

std::optional<hit_record> sphere::hit(const ray& r, const float t_min, const float t_max) const
{
    const displacement oc = r.origin - this->center;
    const float a = glm::dot(r.direction, r.direction);
    const float b = glm::dot(oc, r.direction);
    const float c = glm::dot(oc, oc) - this->radius * this->radius;
    const float discriminant = b * b - a * c;

    if (discriminant > 0.f)
    {
        const auto calc = [this, &r](const float root)
        {
            const position point = r.point_at_parameter(root);
            return hit_record{ root, point, (point - this->center) / radius, this->mat.get() };
        };
        if (const float root_1 = (-b - glm::sqrt(discriminant)) / a; root_1 < t_max && root_1 > t_min)
        {
            return calc(root_1);
        }
        if (const float root_2 = (-b + glm::sqrt(discriminant)) / a; root_2 < t_max && root_2 > t_min)
        {
            return calc(root_2);
        }
    }
    return {};
}