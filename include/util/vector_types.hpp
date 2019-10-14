#pragma once

#include <glm/glm.hpp>

using position = glm::vec3;
using displacement = glm::vec3;
using axis = glm::vec3;

static constexpr axis x_axis = { 1.f, 0.f, 0.f };
static constexpr axis y_axis = { 0.f, 1.f, 0.f };
static constexpr axis z_axis = { 0.f, 0.f, 1.f };