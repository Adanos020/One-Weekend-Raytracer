#pragma once

#include <util/types.hpp>

struct rotator
{
    float pitch;
    float yaw;
    float roll;

    rotator& operator+=(const rotator&);
    rotator& operator-=(const rotator&);
};

inline static rotator operator+(const rotator& a, const rotator& b)
{
    return {
        a.pitch + b.pitch,
        a.yaw + b.yaw,
        a.roll + b.roll,
    };
}

inline static rotator operator-(const rotator& a)
{
    return { -a.pitch, -a.yaw, -a.roll };
}

inline static rotator operator-(const rotator& a, const rotator& b)
{
    return a + -b;
}

struct transform
{
    position origin;
    glm::vec3 scale;
    rotator rotation;

    transform& move_to(const position&);
    transform& move_by(const displacement&);
    transform& scale_to(const glm::vec3&);
    transform& scale_by(const glm::vec3&);
    transform& scale_by(const float);
    transform& rotate_to(const rotator&);
    transform& rotate_by(const rotator&);
};