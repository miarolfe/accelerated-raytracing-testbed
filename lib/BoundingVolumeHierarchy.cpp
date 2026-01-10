// Copyright Mia Rolfe. All rights reserved.
#include <BoundingVolumeHierarchy.h>
#include <RayHitResult.h>

#include <algorithm>

namespace ART
{

BVHNode::BVHNode(std::vector<IRayHittable*>& objects)
    : m_allocator(nullptr), m_left(nullptr), m_right(nullptr)
{
    // BVH has at most 2N-1 nodes for N objects
    const std::size_t arena_size = (2 * objects.size()) * sizeof(BVHNode);
    m_allocator = new ArenaAllocator(arena_size);

    Create(objects.data(), objects.size(), *m_allocator);
}

BVHNode::BVHNode(IRayHittable** objects, std::size_t count, ArenaAllocator& allocator)
    : m_allocator(nullptr), m_left(nullptr), m_right(nullptr)
{
    Create(objects, count, allocator);
}

BVHNode::~BVHNode()
{
    // Only root node owns and deletes allocator
    if (m_allocator)
    {
        delete m_allocator;
        m_allocator = nullptr;
    }
}

void BVHNode::Create(IRayHittable** objects, std::size_t count, ArenaAllocator& allocator)
{
    // Compute bounding box for all objects
    for (std::size_t i = 0; i < count; i++)
    {
        m_bounding_box = AABB(m_bounding_box, objects[i]->BoundingBox());
    }

    const std::size_t axis = m_bounding_box.LongestAxis();

    // Only one object, store directly as leaves
    if (count == 1)
    {
        m_left = objects[0];
        m_right = nullptr;
    }
    // Two objects, store directly as leaves
    else if (count == 2)
    {
        m_left = objects[0];
        m_right = objects[1];
    }
    // Subdivision case
    else
    {
        // Sort objects by centroid along longest axis
        std::sort(objects, objects + count, [axis](IRayHittable* a, IRayHittable* b)
        {
            const Interval& a_interval = a->BoundingBox()[axis];
            const Interval& b_interval = b->BoundingBox()[axis];
            // Cheaper but equivalent for sorting comparison to comparing (2 * a_centroid)to (2 * b_centroid)
            return (a_interval.m_min + a_interval.m_max) < (b_interval.m_min + b_interval.m_max);
        });

        // Split now spatially-sorted objects at midpoint
        std::size_t mid = count / 2;

        m_left = allocator.Create<BVHNode>(objects, mid, allocator);
        m_right = allocator.Create<BVHNode>(objects + mid, count - mid, allocator);
    }
}

bool BVHNode::Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const
{
    if (!m_bounding_box.Hit(ray, ray_t))
    {
        return false;
    }

    // Leaf nodes only have one child (left)
    if (m_right == nullptr)
    {
        return m_left->Hit(ray, ray_t, out_result);
    }

    // Find closest hit of child nodes
    const bool hit_left = m_left->Hit(ray, ray_t, out_result);
    const bool hit_right = m_right->Hit(ray, Interval(ray_t.m_min, hit_left ? out_result.m_t : ray_t.m_max), out_result);

    return hit_left || hit_right;
}

AABB BVHNode::BoundingBox() const
{
    return m_bounding_box;
}

} // namespace ART
