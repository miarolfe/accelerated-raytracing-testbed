// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <AxisAlignedBoundingBox.h>
#include <IRayHittable.h>
#include <Material.h>

namespace ART
{

// Ray-hittable primitive based on AABB
struct AxisAlignedBox : IRayHittable
{
public:
    AABB m_bounding_box;
    Material* m_material;

    AxisAlignedBox(const Point3& min, const Point3& max, Material* material);

    AxisAlignedBox(const AABB& bounding_box, Material* material);

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;
    AABB BoundingBox() const override;
};

} // namespace ART
