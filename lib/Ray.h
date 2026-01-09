// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Vec3.h>

namespace ART
{

class Ray
{
public:
    Point3 m_origin;
    Vec3 m_direction;
    double m_time;

    // Default constructor
    Ray();

    // Constructor without time parameter
    Ray(const Point3& origin, const Vec3& direction);

    // Constructor with time parameter
    Ray(const Point3& origin, const Vec3& direction, double time);

    // Returns the point "t" along the ray
    Point3 At(double t) const;
};

} // namespace ART
