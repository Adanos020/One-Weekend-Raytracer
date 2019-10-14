#include <bounding_volume_hierarchy/bounding_volume_hierarchy_node.hpp>

#include <util/random.hpp>
#include <util/pairs.hpp>

#include <algorithm>
#include <stdexcept>

enum class vector_component
{
    x, y, z
};

template <vector_component coordinate>
static bool compare_boxes(const unique_hittable& a, const unique_hittable& b)
{
    static_assert(coordinate >= vector_component::x && coordinate <= vector_component::z);

    const axis_aligned_bounding_box_opt box_a = a->bounding_box({ 0.f, 0.f });
    const axis_aligned_bounding_box_opt box_b = b->bounding_box({ 0.f, 0.f });

    if (!box_a || !box_b)
    {
        throw std::runtime_error{ "No bounding boxes could be obtained." };
    }

    if constexpr (coordinate == vector_component::x)
    {
        return box_a->min.x < box_b->min.x;
    }
    else if constexpr (coordinate == vector_component::y)
    {
        return box_a->min.y < box_b->min.y;
    }
    else
    {
        return box_a->min.z < box_b->min.z;
    }
}

bounding_volume_hierarchy_node::bounding_volume_hierarchy_node(
    const iterator_pair<std::vector<unique_hittable>> hittables, const min_max<float> time)
{
    const vector_component coordinate_axis = vector_component{ random_uniform<int>(0, 3) };
    if (coordinate_axis == vector_component::x)
    {
        std::stable_sort(hittables.begin, hittables.end, compare_boxes<vector_component::x>);
    }
    else if (coordinate_axis == vector_component::y)
    {
        std::stable_sort(hittables.begin, hittables.end, compare_boxes<vector_component::y>);
    }
    else
    {
        std::stable_sort(hittables.begin, hittables.end, compare_boxes<vector_component::z>);
    }

    if (const size_t count = std::distance(hittables.begin, hittables.end); count == 1)
    {
        this->left = hittables.begin[0].get();
        this->right = hittables.begin[0].get();
    }
    else if (count == 2)
    {
        this->left = hittables.begin[0].get();
        this->right = hittables.begin[1].get();
    }
    else
    {
        // God forgive me these sins
        this->left = new bounding_volume_hierarchy_node{ { hittables.begin, hittables.begin + (count / 2) }, time };
        this->right = new bounding_volume_hierarchy_node{ { hittables.begin + (count / 2), hittables.end }, time };
        this->both_are_bvhs = true;
    }

    const axis_aligned_bounding_box_opt box_left = left->bounding_box(time);
    const axis_aligned_bounding_box_opt box_right = right->bounding_box(time);
    if (box_left && box_right)
    {
        this->box = axis_aligned_bounding_box::surrounding(*box_left, *box_right);
    }
    else
    {
        throw std::runtime_error{ "Couldn't create bounding box for the BVH." };
    }
}

bounding_volume_hierarchy_node::~bounding_volume_hierarchy_node()
{
    if (this->both_are_bvhs)
    {
        // I'm really sorry
        delete this->left;
        delete this->right;

        this->left = nullptr;
        this->right = nullptr;
    }
}

hit_record_opt bounding_volume_hierarchy_node::hit(const struct line& ray, const min_max<float> t) const
{
    if (this->box.hit(ray, t))
    {
        const hit_record_opt hit_left = this->left->hit(ray, t);
        const hit_record_opt hit_right = this->right->hit(ray, t);
        if (hit_left)
        {
            if (hit_right)
            {
                return std::min(*hit_left, *hit_right, [](const hit_record& a, const hit_record& b) { return a.t < b.t; });
            }
            return hit_left;
        }
        return hit_right;
    }
    return {};
}

axis_aligned_bounding_box_opt bounding_volume_hierarchy_node::bounding_box(const min_max<float> t) const
{
    return this->box;
}