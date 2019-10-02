#pragma once

#include <util/types.hpp>

#include <memory>

class texture
{
public:
    virtual ~texture() = default;
    virtual color value_at(const float u, const float v, const position&) const = 0;
};

using unique_texture = std::unique_ptr<texture>;