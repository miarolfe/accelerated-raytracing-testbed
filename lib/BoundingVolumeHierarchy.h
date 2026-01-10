// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Common.h>
#include <IRayHittable.h>
#include <Interval.h>
#include <RayHitResult.h>
#include <Vec3.h>

namespace ART
{

class BoundingVolumeHierarchy : public IRayHittable
{
public:
    BoundingVolumeHierarchy(std::vector<IRayHittable*>& objects);

    ~BoundingVolumeHierarchy();

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    AABB BoundingBox() const override;

protected:
    void Create(std::vector<IRayHittable*>& objects);

    void Destroy();

    AABB m_bounding_box;
};

} // namespace ART
