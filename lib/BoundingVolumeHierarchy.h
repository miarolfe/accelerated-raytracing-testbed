// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <ArenaAllocator.h>
#include <Common.h>
#include <IRayHittable.h>
#include <Interval.h>
#include <RayHitResult.h>
#include <Vec3.h>

namespace ART
{

class BVHNode : public IRayHittable
{
public:
    BVHNode(std::vector<IRayHittable*>& objects);

    ~BVHNode();

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    AABB BoundingBox() const override;

    BVHNode(IRayHittable** objects, std::size_t count, ArenaAllocator& allocator);

protected:
    void Create(IRayHittable** objects, std::size_t count, ArenaAllocator& allocator);

    AABB m_bounding_box;
    ArenaAllocator* m_allocator = nullptr;  // Only root node owns allocator
    IRayHittable* m_left = nullptr;
    IRayHittable* m_right = nullptr;
};

} // namespace ART
