#include <math/transform.hpp>

rotator& rotator::operator+=(const rotator& b)
{
    this->pitch += b.pitch;
    this->yaw += b.yaw;
    this->roll += b.roll;
    return *this;
}

rotator& rotator::operator-=(const rotator& b)
{
    this->pitch -= b.pitch;
    this->yaw -= b.yaw;
    this->roll -= b.roll;
    return *this;
}

transform& transform::move_to(const position& pos)
{
    this->origin = pos;
    return *this;
}

transform& transform::move_by(const displacement& disp)
{
    this->origin += disp;
    return *this;
}

transform& transform::scale_to(const glm::vec3& scale)
{
    this->scale = scale;
    return *this;
}

transform& transform::scale_to(const float scale)
{
    this->scale = glm::vec3{ scale };
    return *this;
}

transform& transform::scale_by(const glm::vec3& scale)
{
    this->scale *= scale;
    return *this;
}

transform& transform::scale_by(const float scale)
{
    this->scale *= scale;
    return *this;
}

transform& transform::rotate_to(const rotator& rotation)
{
    this->rotation = rotation;
    return *this;
}

transform& transform::rotate_by(const rotator& rotation)
{
    this->rotation += rotation;
    return *this;
}