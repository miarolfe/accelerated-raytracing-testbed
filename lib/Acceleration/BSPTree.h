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

// Represents an arbritrary splitting plane
// with equation: normal . p = distance
struct BSPSplitPlane
{
public:
    Vec3 m_normal;      // Plane normal (normalised)
    double m_distance;  // Distance from origin along normal

    BSPSplitPlane() : m_normal(1.0, 0.0, 0.0), m_distance(0.0) {}
    BSPSplitPlane(const Vec3& normal, double distance) : m_normal(normal), m_distance(distance) {}
};

enum class BSPObjectClassification
{
    BACK,
    SPANNING,
    FRONT
};

class BSPTreeNode : public IRayHittable
{
public:
    BSPTreeNode(std::vector<IRayHittable*>& objects);

    ~BSPTreeNode();

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    AABB BoundingBox() const override;

    BSPTreeNode(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator);

protected:
    void Create(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator);

    // Find optimal split plane using surface area heuristic
    bool FindSplitPlane(IRayHittable** objects, std::size_t count, BSPSplitPlane& out_plane);

    // Classify object relative to split plane
    BSPObjectClassification ClassifyObject(const AABB& box, const BSPSplitPlane& plane) const;

    AABB m_bounding_box;
    ArenaAllocator* m_allocator = nullptr;
    IRayHittable* m_front = nullptr;
    IRayHittable* m_back = nullptr;
    BSPSplitPlane m_split_plane;


    static constexpr double NODE_TRAVERSAL_COST = 1.0;
    static constexpr double HITTABLE_INTERSECT_COST = 1.0;
    static constexpr double FP_TOLERANCE = 1e-10;
    static constexpr std::size_t MAX_DEPTH = 20;
    static constexpr std::size_t MAX_OBJECTS_PER_LEAF = 4;
    static constexpr std::size_t NUM_SAH_BUCKETS = 12;
};

} // namespace ART
