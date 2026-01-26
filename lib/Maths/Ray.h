// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Maths/Vec3.h>

namespace ART
{

class Ray
{
public:
    Point3 m_origin;
    Vec3 m_direction;
    Vec3 m_inverse_direction;

    Ray();

    Ray(const Point3& origin, const Vec3& direction);

    // Returns the point "t" along the ray
    Point3 At(double t) const;
};

} // namespace ART
