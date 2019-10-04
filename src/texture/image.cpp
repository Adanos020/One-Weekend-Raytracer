#include <texture/image.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <stdexcept>

image_texture::image_texture(const std::vector<color>& data, const extent_2d<size_t> size)
    : data(data), size(size)
{
    if (this->data.size() != size.width * size.height)
    {
        throw std::runtime_error("Image size don't match the data size.");
    }
}

image_texture::image_texture(std::string_view image_path)
{
    int width, height, n;
    uint8_t* data = stbi_load(image_path.data(), &width, &height, &n, 0);
    this->size = extent_2d<size_t>{ size_t(width), size_t(height) };

    const float normalized_rgb = 1.f / 255.f;
    this->data.resize(size_t(width) * height);
    for (size_t i = 0; i < size_t(width) * height; ++i)
    {
        this->data[i] = normalized_rgb * color{ data[3 * i + 0], data[3 * i + 1], data[3 * i + 2] };
    }

    stbi_image_free(data);
}

color image_texture::value_at(const std::pair<float, float> uv, const position&) const
{
    const auto [u, v] = uv;
    const size_t i = std::clamp<size_t>(u * float(this->size.width), 0, this->size.width - 1);
    const size_t j = std::clamp<size_t>((1.f - v) * float(this->size.height) - 0.001f, 0, this->size.height - 1);
    return this->data[i + j * this->size.width];
}