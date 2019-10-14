#pragma once

#include <glm/glm.hpp>

using color = glm::vec3;
using color_alpha = glm::vec4;
using rgb = glm::u8vec3;
using rgba = glm::u8vec4;

static constexpr color white = color{ 1.f };
static constexpr color black = color{ 0.f };
static constexpr color red = color{ 1.f, 0.f, 0.f };
static constexpr color green = color{ 0.f, 1.f, 0.f };
static constexpr color blue = color{ 0.f, 0.f, 1.f };
static constexpr color yellow = color{ 1.f, 1.f, 0.f };
static constexpr color magenta = color{ 1.f, 0.f, 1.f };
static constexpr color cyan = color{ 0.f, 1.f, 1.f };

inline static rgb to_rgb(const color& col)
{
    return col * 255.99f;
}

inline static rgba to_rgba(const color_alpha& col)
{
    return col * 255.99f;
}