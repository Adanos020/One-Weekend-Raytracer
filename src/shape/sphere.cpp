#include <shape/sphere.hpp>

#include <ray.hpp>

sphere::sphere(const position& center, const float radius, unique_material&& mat)
    : center(center)
    , radius(radius)
    , mat(std::move(mat))
{
}

hit_record_opt sphere::hit(const ray& r, const min_max<float> t) const
{
    const displacement oc = r.origin - this->center;
    const float a = glm::dot(r.direction, r.direction);
    const float b = glm::dot(oc, r.direction);
    const float c = glm::dot(oc, oc) - this->radius * this->radius;
    const float discriminant = b * b - a * c;

    if (discriminant > 0.f)
    {
        if (const float root_1 = (-b - glm::sqrt(discriminant)) / a; root_1 < t.max && root_1 > t.min)
        {
            const position point = r.point_at_parameter(root_1);
            return hit_record{ root_1, point, (point - this->center) / radius, this->mat.get() };
        }
        if (const float root_2 = (-b + glm::sqrt(discriminant)) / a; root_2 < t.max && root_2 > t.min)
        {
            const position point = r.point_at_parameter(root_2);
            return hit_record{ root_2, point, (point - this->center) / radius, this->mat.get() };
        }
    }
    return {};
}

axis_aligned_bounding_box_opt sphere::bounding_box(const min_max<float> t) const
{
    return axis_aligned_bounding_box{
        position{ this->center - displacement{ this->radius } },
        position{ this->center + displacement{ this->radius } }
    };
}
