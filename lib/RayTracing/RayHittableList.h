// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <vector>

#include <RayTracing/IRayHittable.h>
#include <RayTracing/RayHitResult.h>

namespace ART
{

class RayHittableList : public IRayHittable
{
public:
    RayHittableList();

    RayHittableList(IRayHittable* hittable);

    void Clear();

    void Add(IRayHittable* hittable);

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    std::vector<IRayHittable*>& GetObjects();

    AABB BoundingBox() const override;

protected:
    std::vector<IRayHittable*> m_objects;
    AABB m_bounding_box;
};

} // namespace ART
