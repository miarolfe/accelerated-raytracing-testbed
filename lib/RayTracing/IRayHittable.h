// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Geometry/AxisAlignedBoundingBox.h>
#include <Maths/Interval.h>
#include <Maths/Ray.h>
#include <RayTracing/RayHitResult.h>

namespace ART
{

// Interface base class for objects that can be intersected by rays
struct IRayHittable
{
public:
    virtual ~IRayHittable() = default;
    virtual bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const = 0;
    virtual AABB BoundingBox() const = 0;
};

} // namespace ART
