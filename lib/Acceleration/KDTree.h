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

class KDTreeNode : public IRayHittable
{
public:
    KDTreeNode(std::vector<IRayHittable*>& objects);

    ~KDTreeNode();

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    AABB BoundingBox() const override;

    KDTreeNode(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator);

protected:
    void Create(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator);

    AABB m_bounding_box;
    // Only root node owns allocator
    ArenaAllocator* m_allocator = nullptr;
    IRayHittable* m_left = nullptr;
    IRayHittable* m_right = nullptr;
    // Split axis (0=x, 1=y, 2=z) and position for internal nodes
    std::size_t m_split_axis = 0;
    double m_split_position_along_split_axis = 0.0;

    static constexpr std::size_t MAX_DEPTH = 20;
};

} // namespace ART
