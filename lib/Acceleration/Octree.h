// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Core/ArenaAllocator.h>
#include <Core/Common.h>
#include <Maths/Interval.h>
#include <Maths/Vec3.h>
#include <RayTracing/IRayHittable.h>
#include <RayTracing/RayHitResult.h>

namespace ART
{

class OctreeNode : public IRayHittable
{
public:
    OctreeNode(std::vector<IRayHittable*>& objects);

    ~OctreeNode();

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    AABB BoundingBox() const override;

    std::size_t MemoryUsedBytes() const;

    OctreeNode(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator);

protected:
    void Create(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator);

    std::size_t GetOctant(const AABB& box) const;

    AABB m_bounding_box;
    ArenaAllocator* m_allocator = nullptr;
    IRayHittable* m_children[8] = {nullptr};
    Point3 m_split_centre;
    std::size_t m_leaf_count = 0;

    static constexpr std::size_t MAX_DEPTH = 20;
    static constexpr std::size_t MAX_OBJECTS_PER_LEAF = 4;
};

} // namespace ART
