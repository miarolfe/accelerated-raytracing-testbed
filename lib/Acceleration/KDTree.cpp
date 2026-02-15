// Copyright Mia Rolfe. All rights reserved.
#include <Acceleration/KDTree.h>

#include <algorithm>

#include <Core/TraversalStats.h>

namespace ART
{

KDTreeNode::KDTreeNode(std::vector<IRayHittable*>& objects)
    : m_allocator(nullptr), m_left(nullptr), m_right(nullptr)
{
    // KD-tree has at most 2N-1 nodes for N objects
    // Additional space needed for centroid arrays used in construction
    const std::size_t node_space = (2 * objects.size()) * sizeof(KDTreeNode);
    const std::size_t centroid_space = objects.size() * sizeof(double) * (MAX_DEPTH + 1);
    const std::size_t arena_size = node_space + centroid_space;
    m_allocator = new ArenaAllocator(arena_size);

    Create(objects.data(), objects.size(), 0, *m_allocator);
}

KDTreeNode::KDTreeNode(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator)
    : m_allocator(nullptr), m_left(nullptr), m_right(nullptr)
{
    Create(objects, count, depth, allocator);
}

KDTreeNode::~KDTreeNode()
{
    // Only root node owns and deletes allocator
    if (m_allocator)
    {
        delete m_allocator;
        m_allocator = nullptr;
    }
}

void KDTreeNode::Create(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator)
{
    // Compute bounding box for all objects
    for (std::size_t object_index = 0; object_index < count; object_index++)
    {
        m_bounding_box = AABB(m_bounding_box, objects[object_index]->BoundingBox());
    }

    // Only one object, store directly as leaf
    if (count == 1)
    {
        m_left = objects[0];
        m_right = nullptr;
        return;
    }

    // Two objects, store directly as leaves
    if (count == 2)
    {
        m_left = objects[0];
        m_right = objects[1];
        return;
    }

    // Reached max tree depth, split in half
    if (depth >= MAX_DEPTH)
    {
        const std::size_t mid = count / 2;
        m_left = allocator.Create<KDTreeNode>(objects, mid, depth + 1, allocator);
        m_right = allocator.Create<KDTreeNode>(objects + mid, count - mid, depth + 1, allocator);
        return;
    }

    // Split axis cycles through axes based on depth
    m_split_axis = depth % 3;

    // Find median split position using centroids
    double* centroids = static_cast<double*>(allocator.Alloc(count * sizeof(double), alignof(double)));
    for (std::size_t object_index = 0; object_index < count; object_index++)
    {
        const Interval interval = objects[object_index]->BoundingBox()[m_split_axis];
        centroids[object_index] = 0.5 * (interval.m_min + interval.m_max);
    }
    std::sort(centroids, centroids + count);

    m_split_position_along_split_axis = centroids[count / 2];

    // Partition objects on split position
    IRayHittable** mid = std::partition
    (
        objects, objects + count,
        [this](IRayHittable* obj)
        {
            const Interval interval = obj->BoundingBox()[m_split_axis];
            return (0.5 * (interval.m_min + interval.m_max)) < m_split_position_along_split_axis;
        }
    );

    std::size_t split_index = static_cast<std::size_t>(mid - objects);

    // If all objects are on side of the split, fallback to middle division
    if (split_index == 0 || split_index >= count)
    {
        split_index = count / 2;
    }

    m_left = allocator.Create<KDTreeNode>(objects, split_index, depth + 1, allocator);
    m_right = allocator.Create<KDTreeNode>(objects + split_index, count - split_index, depth + 1, allocator);
}

bool KDTreeNode::Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const
{
    // First check if ray intersects our bounding box
    if (!m_bounding_box.Hit(ray, ray_t))
    {
        return false;
    }

    RecordNodeTraversal();

    // Leaf node with only one child
    if (m_right == nullptr)
    {
        return m_left->Hit(ray, ray_t, out_result);
    }

    // Determine traversal order based on ray direction
    const double ray_origin_along_axis = ray.m_origin[m_split_axis];
    const double ray_direction_along_axis = ray.m_direction[m_split_axis];

    // If ray is going in negative direction along split axis, or
    // if ray origin is past split position, swap order
    const bool should_swap_child_order = (ray_direction_along_axis < 0.0) ||
        (ray_direction_along_axis == 0.0 && ray_origin_along_axis > m_split_position_along_split_axis);

    IRayHittable* first_child = should_swap_child_order ? m_right : m_left;
    IRayHittable* second_child = should_swap_child_order ? m_left : m_right;

    // Find closest hit of child nodes
    const bool hit_left = first_child->Hit(ray, ray_t, out_result);
    const bool hit_right = second_child->Hit(ray, Interval(ray_t.m_min, hit_left ? out_result.m_t : ray_t.m_max), out_result);

    return hit_left || hit_right;
}

AABB KDTreeNode::BoundingBox() const
{
    return m_bounding_box;
}

std::size_t KDTreeNode::MemoryUsedBytes() const
{
    return m_allocator ? m_allocator->MemoryUsedBytes() : 0;
}

} // namespace ART
