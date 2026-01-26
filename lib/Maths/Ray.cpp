// Copyright Mia Rolfe. All rights reserved.
#include <Maths/Ray.h>

namespace ART
{

Ray::Ray() : m_origin(0.0), m_direction(0.0), m_inverse_direction(0.0) {}

Ray::Ray(const Point3& origin, const Vec3& direction)
    : m_origin(origin), m_direction(direction),
      m_inverse_direction(1.0 / direction.m_x, 1.0 / direction.m_y, 1.0 / direction.m_z) {}

Point3 Ray::At(double t) const
{
    return m_origin + (m_direction * t);
}

} // namespace ART
