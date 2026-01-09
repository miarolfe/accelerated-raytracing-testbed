// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <AxisAlignedBoundingBox.h>
#include <Interval.h>
#include <Ray.h>
#include <RayHitResult.h>

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
