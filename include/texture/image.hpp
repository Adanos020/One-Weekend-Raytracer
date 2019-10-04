#pragma once

#include <texture.hpp>
#include <util/types.hpp>

#include <string>
#include <vector>

class image_texture : public texture
{
public:
    image_texture() = default;
    image_texture(std::string_view image_path);
    image_texture(const std::vector<color>& data, const extent_2d<size_t> size);
    virtual color value_at(const std::pair<float, float> uv, const position&) const override;

private:
    std::vector<color> data;
    extent_2d<size_t> size;
};