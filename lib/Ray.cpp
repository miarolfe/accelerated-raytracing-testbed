// Copyright Mia Rolfe. All rights reserved.
#include "Ray.h"

namespace ART
{

Ray::Ray()
{
    m_time = 0.0;
}

Ray::Ray(const Point3& origin, const Vec3& direction)
{
    m_origin = origin;
    m_direction = direction;
    m_time = 0.0;
}

Ray::Ray(const Point3& origin, const Vec3& direction, double time)
{
    m_origin = origin;
    m_direction = direction;
    m_time = time;
}

Point3 Ray::At(double t) const
{
    return m_origin + (m_direction * t);
};

} // namespace ART
