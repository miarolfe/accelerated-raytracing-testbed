// Copyright Mia Rolfe. All rights reserved.
#include <Acceleration/BSPTree.h>

#include <Core/Common.h>

namespace ART
{

BSPTreeNode::BSPTreeNode(std::vector<IRayHittable*>& objects)
    : m_allocator(nullptr), m_front(nullptr), m_back(nullptr)
{
    // Worst-case highwater-mark guess, every object duplicated into both child nodes
    const std::size_t arena_size = (4 * objects.size()) * sizeof(BSPTreeNode);
    m_allocator = new ArenaAllocator(arena_size);

    Create(objects.data(), objects.size(), 0, *m_allocator);
}

BSPTreeNode::BSPTreeNode(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator)
    : m_allocator(nullptr), m_front(nullptr), m_back(nullptr)
{
    Create(objects, count, depth, allocator);
}

BSPTreeNode::~BSPTreeNode()
{
    // Only root node owns and deletes allocator
    if (m_allocator)
    {
        delete m_allocator;
        m_allocator = nullptr;
    }
}

BSPObjectClassification BSPTreeNode::ClassifyObject(const AABB& box, const BSPSplitPlane& plane) const
{
    // Project AABB extents onto plane normal to find min/max signed distances
    const Vec3& normal = plane.m_normal;
    const Point3 centre
    (
        0.5 * (box.m_x.m_min + box.m_x.m_max),
        0.5 * (box.m_y.m_min + box.m_y.m_max),
        0.5 * (box.m_z.m_min + box.m_z.m_max)
    );
    const Vec3 half_extents
    (
        0.5 * (box.m_x.m_max - box.m_x.m_min),
        0.5 * (box.m_y.m_max - box.m_y.m_min),
        0.5 * (box.m_z.m_max - box.m_z.m_min)
    );

    const double distance_from_centre_to_plane = Dot(normal, centre) - plane.m_distance;

    // Maximum extent along normal direction
    const double radius_x = std::abs(normal.m_x) * half_extents.m_x;
    const double radius_y = std::abs(normal.m_y) * half_extents.m_y;
    const double radius_z = std::abs(normal.m_z) * half_extents.m_z;
    const double radius = radius_x + radius_y + radius_z;

    // Object's bounding box is entirely in front of splitting plane
    if (distance_from_centre_to_plane > (radius + FP_TOLERANCE))
    {
        return BSPObjectClassification::FRONT;
    }

    // Object's bounding box is entirely behind splitting plane
    if (distance_from_centre_to_plane < (-radius - FP_TOLERANCE))
    {
        return BSPObjectClassification::BACK;
    }

    // Object's bounding box spans the splitting plane
    return BSPObjectClassification::SPANNING;
}

bool BSPTreeNode::FindSplitPlane(IRayHittable** objects, std::size_t count, BSPSplitPlane& out_plane)
{
    const double parent_node_surface_area = m_bounding_box.SurfaceArea();
    const double leaf_cost = count * HITTABLE_INTERSECT_COST;
    double best_cost = std::numeric_limits<double>::max();
    BSPSplitPlane best_splitting_plane;

    // 3 axis-aligned (like k-d tree) + 4 diagonals
    const Vec3 normals[7] =
    {
        Vec3(1.0, 0.0, 0.0),
        Vec3(0.0, 1.0, 0.0),
        Vec3(0.0, 0.0, 1.0),
        Normalised(Vec3(1.0, 1.0, 0.0)),
        Normalised(Vec3(1.0, 0.0, 1.0)),
        Normalised(Vec3(0.0, 1.0, 1.0)),
        Normalised(Vec3(1.0, 1.0, 1.0))
    };

    for (std::size_t normal_index = 0; normal_index < 7; normal_index++)
    {
        const Vec3& normal = normals[normal_index];

        // Project every object's centroid onto the candidate plane's normal
        // to find the total range of split positions
        double min_projection = std::numeric_limits<double>::max();
        double max_projection = std::numeric_limits<double>::lowest();

        for (std::size_t object_index = 0; object_index < count; object_index++)
        {
            const AABB& box = objects[object_index]->BoundingBox();

            const double projection_x = normal.m_x * (box.m_x.m_min + box.m_x.m_max);
            const double projection_y = normal.m_y * (box.m_y.m_min + box.m_y.m_max);
            const double projection_z = normal.m_z * (box.m_z.m_min + box.m_z.m_max);
            const double projection = projection_x + projection_y + projection_z;

            min_projection = std::min(min_projection, projection);
            max_projection = std::max(max_projection, projection);
        }

        const double projection_extent = max_projection - min_projection;
        if (projection_extent < FP_TOLERANCE)
        {
            continue;
        }

        // Evaluate bucket split positions
        for (std::size_t split = 1; split < NUM_SAH_BUCKETS; split++)
        {
            const double split_distance = 0.5 * (min_projection + (split * projection_extent / NUM_SAH_BUCKETS));
            BSPSplitPlane test_plane(normal, split_distance);

            AABB front_bounding_box;
            AABB back_bounding_box;
            std::size_t front_num_objects = 0;
            std::size_t back_num_objects = 0;

            for (std::size_t object_index = 0; object_index < count; object_index++)
            {
                const AABB& object_bounding_box = objects[object_index]->BoundingBox();
                const BSPObjectClassification classification = ClassifyObject(object_bounding_box, test_plane);

                if (classification >= BSPObjectClassification::SPANNING)
                {
                    front_bounding_box = AABB(front_bounding_box, object_bounding_box);
                    front_num_objects++;
                }
                if (classification <= BSPObjectClassification::SPANNING)
                {
                    back_bounding_box = AABB(back_bounding_box, object_bounding_box);
                    back_num_objects++;
                }
            }

            if (front_num_objects == 0 || back_num_objects == 0)
            {
                continue;
            }

            const double cost_of_front_subtree = (front_bounding_box.SurfaceArea() / parent_node_surface_area) * front_num_objects * HITTABLE_INTERSECT_COST;
            const double cost_of_back_subtree = (back_bounding_box.SurfaceArea() / parent_node_surface_area) * back_num_objects * HITTABLE_INTERSECT_COST;
            const double total_cost = NODE_TRAVERSAL_COST + cost_of_front_subtree + cost_of_back_subtree;

            if (total_cost < best_cost)
            {
                best_cost = total_cost;
                best_splitting_plane = test_plane;
            }
        }
    }

    // No worthwhile split found
    if (best_cost >= leaf_cost)
    {
        return false;
    }

    out_plane = best_splitting_plane;
    return true;
}

void BSPTreeNode::Create(IRayHittable** objects, std::size_t count, std::size_t depth, ArenaAllocator& allocator)
{
    // Compute bounding box
    for (std::size_t object_index = 0; object_index < count; object_index++)
    {
        m_bounding_box = AABB(m_bounding_box, objects[object_index]->BoundingBox());
    }

    // Create leaf if small number of objects left, hit max depth, or no good split found
    if (count <= MAX_OBJECTS_PER_LEAF || depth >= MAX_DEPTH || !FindSplitPlane(objects, count, m_split_plane))
    {
        m_front = objects[0];
        m_back = (count > 1) ? allocator.Create<BSPTreeNode>(objects + 1, count - 1, depth + 1, allocator) : nullptr;
        return;
    }

    std::size_t front_num_objects = 0;
    std::size_t back_num_objects = 0;
    for (std::size_t object_index = 0; object_index < count; object_index++)
    {
        const BSPObjectClassification classification = ClassifyObject(objects[object_index]->BoundingBox(), m_split_plane);
        front_num_objects += static_cast<std::size_t>(classification >= BSPObjectClassification::SPANNING);
        back_num_objects += static_cast<std::size_t>(classification <= BSPObjectClassification::SPANNING);
    }

    // If all objects on one side of the split, just split in half
    if (front_num_objects == 0 || back_num_objects == 0 || (front_num_objects == count && back_num_objects == count))
    {
        const std::size_t mid_index = count / 2;
        m_front = allocator.Create<BSPTreeNode>(objects, mid_index, depth + 1, allocator);
        m_back = allocator.Create<BSPTreeNode>(objects + mid_index, count - mid_index, depth + 1, allocator);
        return;
    }

    // Allocate and distribute objects
    IRayHittable** front_objects = static_cast<IRayHittable**>
    (
        allocator.Alloc(front_num_objects * sizeof(IRayHittable*), alignof(IRayHittable*))
    );
    IRayHittable** back_objects = static_cast<IRayHittable**>
    (
        allocator.Alloc(back_num_objects * sizeof(IRayHittable*), alignof(IRayHittable*))
    );

    std::size_t front_objects_index = 0;
    std::size_t back_objects_index = 0;
    for (std::size_t object_index = 0; object_index < count; object_index++)
    {
        const BSPObjectClassification classification = ClassifyObject(objects[object_index]->BoundingBox(), m_split_plane);
        if (classification >= BSPObjectClassification::SPANNING)
        {
            front_objects[front_objects_index] = objects[object_index];
            front_objects_index++;
        }
        if (classification <= BSPObjectClassification::SPANNING)
        {
            back_objects[back_objects_index] = objects[object_index];
            back_objects_index++;
        }
    }

    m_front = allocator.Create<BSPTreeNode>(front_objects, front_num_objects, depth + 1, allocator);
    m_back = allocator.Create<BSPTreeNode>(back_objects, back_num_objects, depth + 1, allocator);
}

bool BSPTreeNode::Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const
{
    if (!m_bounding_box.Hit(ray, ray_t))
    {
        return false;
    }

    // Single child leaf
    if (m_back == nullptr)
    {
        return m_front->Hit(ray, ray_t, out_result);
    }

    // Determine traversal order (find closest side to origin)
    const double origin_distance = Dot(m_split_plane.m_normal, ray.m_origin) - m_split_plane.m_distance;
    const bool origin_in_front = origin_distance >= 0.0;

    IRayHittable* first = origin_in_front ? m_front : m_back;
    IRayHittable* second = origin_in_front ? m_back : m_front;

    const bool hit_first = first->Hit(ray, ray_t, out_result);
    const bool hit_second = second->Hit(ray, Interval(ray_t.m_min, hit_first ? out_result.m_t : ray_t.m_max), out_result);

    return hit_first || hit_second;
}

AABB BSPTreeNode::BoundingBox() const
{
    return m_bounding_box;
}

std::size_t BSPTreeNode::MemoryUsedBytes() const
{
    return m_allocator ? m_allocator->MemoryUsedBytes() : 0;
}

} // namespace ART
