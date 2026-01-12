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

struct SplitBucket
{
public:
    AABB bounding_box;
    std::size_t num_hittables = 0;
};

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

    // Split objects using surface-area heuristic
    // Returns index of split, 0 if no beneficial split found
    std::size_t SplitSAH(IRayHittable** objects, std::size_t count);

    // Fallback if SplitSAH couldn't find good split
    std::size_t SplitLongestAxis(IRayHittable** objects, std::size_t count);

    AABB m_bounding_box;
    // Only root node owns allocator
    ArenaAllocator* m_allocator = nullptr;
    IRayHittable* m_left = nullptr;
    IRayHittable* m_right = nullptr;

    static constexpr double NODE_TRAVERSAL_COST = 1.0;
    static constexpr double HITTABLE_INTERSECT_COST = 1.0;
    static constexpr std::size_t NUM_SAH_BUCKETS = 12;
};

} // namespace ART
