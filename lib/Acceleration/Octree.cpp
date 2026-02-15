// Copyright Mia Rolfe. All rights reserved.
#include <Acceleration/Octree.h>

#include <Core/TraversalStats.h>

namespace ART
{

OctreeNode::OctreeNode(std::vector<IRayHittable*>& objects)
    : m_allocator(nullptr)
{
    const std::size_t arena_size = (8 * objects.size()) * sizeof(OctreeNode);
    m_allocator = new ArenaAllocator(arena_size);

    Create(objects.data(), objects.size(), 0, *m_allocator);
}

OctreeNode::OctreeNode(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator)
    : m_allocator(nullptr)
{
    Create(objects, count, depth, allocator);
}

OctreeNode::~OctreeNode()
{
    // Only root node owns the allocator
    if (m_allocator)
    {
        delete m_allocator;
        m_allocator = nullptr;
    }
}

// Returns octant index (0-7) based on which side of split centre the box's centroid lies
// Bit 0: x >= split
// Bit 1: y >= split
// Bit 2: z >= split
std::size_t OctreeNode::GetOctant(const AABB& bounding_box) const
{
    // Using sum instead of average avoids division (centroid = (min+max)/2)
    const double centroid_x = bounding_box.m_x.m_min + bounding_box.m_x.m_max;
    const double centroid_y = bounding_box.m_y.m_min + bounding_box.m_y.m_max;
    const double centroid_z = bounding_box.m_z.m_min + bounding_box.m_z.m_max;

    const std::size_t x_bit = static_cast<std::size_t>(centroid_x >= 2.0 * m_split_centre.m_x);
    const std::size_t y_bit = static_cast<std::size_t>(centroid_y >= 2.0 * m_split_centre.m_y);
    const std::size_t z_bit = static_cast<std::size_t>(centroid_z >= 2.0 * m_split_centre.m_z);

    return (x_bit | (y_bit << 1) | (z_bit << 2));
}

void OctreeNode::Create(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator)
{
    // Compute bounding box for this node
    for (std::size_t object_index = 0; object_index < count; object_index++)
    {
        m_bounding_box = AABB(m_bounding_box, objects[object_index]->BoundingBox());
    }

    // Split point must be at centre of bounding box
    m_split_centre = Point3
    (
        0.5 * (m_bounding_box.m_x.m_min + m_bounding_box.m_x.m_max),
        0.5 * (m_bounding_box.m_y.m_min + m_bounding_box.m_y.m_max),
        0.5 * (m_bounding_box.m_z.m_min + m_bounding_box.m_z.m_max)
    );

    // Create leaf node if at max depth or object density low enough
    if (count <= MAX_OBJECTS_PER_LEAF || depth >= MAX_DEPTH)
    {
        m_leaf_count = count;
        for (std::size_t i = 0; i < count; i++)
        {
            m_children[i] = objects[i];
        }
        return;
    }

    std::size_t object_count_per_octant[8] = {0};
    for (std::size_t object_index = 0; object_index < count; object_index++)
    {
        object_count_per_octant[GetOctant(objects[object_index]->BoundingBox())]++;
    }

    // If all objects are in same octant, create leaf
    std::size_t num_occupied_octants = 0;
    for (std::size_t octant_index = 0; octant_index < 8; octant_index++)
    {
        num_occupied_octants += static_cast<std::size_t>(object_count_per_octant[octant_index] > 0);
    }
    if (num_occupied_octants <= 1)
    {
        m_leaf_count = count;
        for (std::size_t object_index = 0; object_index < count; object_index++)
        {
            m_children[object_index] = objects[object_index];
        }
        return;
    }

    std::size_t octant_start_offsets[8];
    octant_start_offsets[0] = 0;
    for (std::size_t i = 1; i < 8; i++)
    {
        octant_start_offsets[i] = octant_start_offsets[i - 1] + object_count_per_octant[i - 1];
    }

    // Distribute objects by octant
    std::size_t octant_next_free_index[8];
    for (std::size_t i = 0; i < 8; i++)
    {
        octant_next_free_index[i] = octant_start_offsets[i];
    }

    IRayHittable** objects_sorted_by_octant = static_cast<IRayHittable**>
    (
        allocator.Alloc(count * sizeof(IRayHittable*), alignof(IRayHittable*))
    );

    for (std::size_t object_index = 0; object_index < count; object_index++)
    {
        const std::size_t octant = GetOctant(objects[object_index]->BoundingBox());
        objects_sorted_by_octant[octant_next_free_index[octant]++] = objects[object_index];
    }

    // Create child nodes
    for (std::size_t octant = 0; octant < 8; octant++)
    {
        if (object_count_per_octant[octant] > 0)
        {
            m_children[octant] = allocator.Create<OctreeNode>
            (
                objects_sorted_by_octant + octant_start_offsets[octant],
                object_count_per_octant[octant],
                depth + 1,
                allocator
            );
        }
    }
}

bool OctreeNode::Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const
{
    if (!m_bounding_box.Hit(ray, ray_t))
    {
        return false;
    }

    RecordNodeTraversal();

    // Leaf node: test stored objects
    if (m_leaf_count > 0)
    {
        bool hit_anything = false;
        double closest_so_far = ray_t.m_max;

        for (std::size_t object_index = 0; object_index < m_leaf_count; object_index++)
        {
            if (m_children[object_index]->Hit(ray, Interval(ray_t.m_min, closest_so_far), out_result))
            {
                hit_anything = true;
                closest_so_far = out_result.m_t;
            }
        }
        return hit_anything;
    }

    const std::size_t direction_x_is_negative = static_cast<std::size_t>(ray.m_direction.m_x < 0);
    const std::size_t direction_y_is_negative = static_cast<std::size_t>(ray.m_direction.m_y < 0);
    const std::size_t direction_z_is_negative = static_cast<std::size_t>(ray.m_direction.m_z < 0);
    const std::size_t direction_mask = direction_x_is_negative | (direction_y_is_negative << 1) | (direction_z_is_negative << 2);

    bool hit_anything = false;
    double closest_so_far = ray_t.m_max;

    for (std::size_t i = 0; i < 8; i++)
    {
        const std::size_t octant = i ^ direction_mask;
        if (m_children[octant] && m_children[octant]->Hit(ray, Interval(ray_t.m_min, closest_so_far), out_result))
        {
            hit_anything = true;
            closest_so_far = out_result.m_t;
        }
    }

    return hit_anything;
}

AABB OctreeNode::BoundingBox() const
{
    return m_bounding_box;
}

std::size_t OctreeNode::MemoryUsedBytes() const
{
    return m_allocator ? m_allocator->MemoryUsedBytes() : 0;
}

} // namespace ART
