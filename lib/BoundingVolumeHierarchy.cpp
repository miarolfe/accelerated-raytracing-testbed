// Copyright Mia Rolfe. All rights reserved.
#include <BoundingVolumeHierarchy.h>

#include <algorithm>
#include <limits>

#include <IRayHittable.h>
#include <RayHitResult.h>

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

    std::size_t split = SplitSAH(objects, count);

    // If SAH split fails use fallback method
    if (split == 0 || split >= count)
    {
        split = SplitLongestAxis(objects, count);
    }

    m_left = allocator.Create<BVHNode>(objects, split, allocator);
    m_right = allocator.Create<BVHNode>(objects + split, count - split, allocator);
}

std::size_t BVHNode::SplitSAH(IRayHittable** objects, std::size_t count)
{
    const double parent_node_surface_area = m_bounding_box.SurfaceArea();
    const double leaf_cost = count * HITTABLE_INTERSECT_COST;

    double best_cost = std::numeric_limits<double>::max();
    double best_split_pos_along_best_axis = 0.0;
    std::size_t best_axis = 0;
    std::size_t best_bucket = 0;

    // Evaluate SAH for each axis
    for (std::size_t axis = 0; axis < 3; axis++)
    {
        double min_centroid = std::numeric_limits<double>::max();
        double max_centroid = std::numeric_limits<double>::lowest();

        for (std::size_t object_index = 0; object_index < count; object_index++)
        {
            const Interval& interval = objects[object_index]->BoundingBox()[axis];
            const double centroid = 0.5 * (interval.m_min + interval.m_max);
            min_centroid = std::min(min_centroid, centroid);
            max_centroid = std::max(max_centroid, centroid);
        }

        const double extent = max_centroid - min_centroid;
        static constexpr double fp_tolerance = 1e-10;
        if (extent < fp_tolerance)
        {
            continue;
        }

        // Assign objects to buckets
        SplitBucket buckets[NUM_SAH_BUCKETS];
        for (std::size_t object_index = 0; object_index < count; object_index++)
        {
            const Interval& interval = objects[object_index]->BoundingBox()[axis];
            const double centroid = 0.5 * (interval.m_min + interval.m_max);
            std::size_t bucket_index = static_cast<std::size_t>(NUM_SAH_BUCKETS * ((centroid - min_centroid) / extent));
            if (bucket_index >= NUM_SAH_BUCKETS)
            {
                bucket_index = NUM_SAH_BUCKETS - 1;
            }
            buckets[bucket_index].num_hittables++;
            buckets[bucket_index].bounding_box = AABB(buckets[bucket_index].bounding_box, objects[object_index]->BoundingBox());
        }

        // Evaluate split positions
        for (std::size_t split = 1; split < NUM_SAH_BUCKETS; split++)
        {
            AABB left_bounding_box;
            AABB right_bounding_box;
            std::size_t left_num_hittables = 0;
            std::size_t right_num_hittables = 0;

            for (std::size_t bucket_index = 0; bucket_index < split; bucket_index++)
            {
                if (buckets[bucket_index].num_hittables > 0)
                {
                    left_bounding_box = AABB(left_bounding_box, buckets[bucket_index].bounding_box);
                    left_num_hittables += buckets[bucket_index].num_hittables;
                }
            }
            for (std::size_t bucket_index = split; bucket_index < NUM_SAH_BUCKETS; bucket_index++)
            {
                if (buckets[bucket_index].num_hittables > 0)
                {
                    right_bounding_box = AABB(right_bounding_box, buckets[bucket_index].bounding_box);
                    right_num_hittables += buckets[bucket_index].num_hittables;
                }
            }

            if (left_num_hittables == 0 || right_num_hittables == 0) continue;

            const double cost_of_left_subtree = (left_bounding_box.SurfaceArea() / parent_node_surface_area) * left_num_hittables * HITTABLE_INTERSECT_COST;
            const double cost_of_right_subtree = (right_bounding_box.SurfaceArea() / parent_node_surface_area) * right_num_hittables * HITTABLE_INTERSECT_COST;
            const double total_cost = NODE_TRAVERSAL_COST + cost_of_left_subtree + cost_of_right_subtree;

            if (total_cost < best_cost)
            {
                best_cost = total_cost;
                best_axis = axis;
                best_bucket = split;
                best_split_pos_along_best_axis = min_centroid + (split * extent / NUM_SAH_BUCKETS);
            }
        }
    }

    // No worthwhile split found
    if (best_cost >= leaf_cost)
    {
        return 0;
    }

    // Partition objects at the best split position
    IRayHittable** mid = std::partition
    (
        objects, objects + count,
        [best_axis, best_split_pos_along_best_axis](IRayHittable* obj)
        {
            const Interval& interval = obj->BoundingBox()[best_axis];
            return 0.5 * (interval.m_min + interval.m_max) < best_split_pos_along_best_axis;
        }
    );

    const std::size_t split_index = static_cast<std::size_t>(mid - objects);
    return split_index;
}

std::size_t BVHNode::SplitLongestAxis(IRayHittable** objects, std::size_t count)
{
    const std::size_t axis = m_bounding_box.LongestAxis();

    std::sort(objects, objects + count, [axis](IRayHittable* a, IRayHittable* b)
    {
        const Interval& a_interval = a->BoundingBox()[axis];
        const Interval& b_interval = b->BoundingBox()[axis];
        // Cheaper but equivalent for sorting comparison to comparing (2 * a_centroid) to (2 * b_centroid)
        return (a_interval.m_min + a_interval.m_max) < (b_interval.m_min + b_interval.m_max);
    });

    const std::size_t split_index = count / 2;
    return split_index;
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
