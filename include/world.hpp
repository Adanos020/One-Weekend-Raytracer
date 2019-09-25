#pragma once

#include <hittable.hpp>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

class world : public hittable
{
public:
    template<class T, typename... Args>
    void spawn_object(Args... args)
    {
        static_assert(std::is_base_of_v<hittable, T>);
        this->hittables.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    virtual hit_record_opt hit(const struct ray&, const float t_min, const float t_max) const override;

private:
    std::vector<unique_hittable> hittables;
};