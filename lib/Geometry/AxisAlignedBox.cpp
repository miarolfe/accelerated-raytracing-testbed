// Copyright Mia Rolfe. All rights reserved.
#include <Geometry/AxisAlignedBox.h>

#include <Core/TraversalStats.h>

namespace ART
{

AxisAlignedBox::AxisAlignedBox(const Point3& min, const Point3& max, Material* material)
    : m_bounding_box(min, max), m_material(material) {}

AxisAlignedBox::AxisAlignedBox(const AABB& bounding_box, Material* material)
    : m_bounding_box(bounding_box), m_material(material) {}

bool AxisAlignedBox::Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const
{
    RecordIntersectionTest();

    // Latest entry point across all slabs
    double t_min = ray_t.m_min;

    // Earliest exit point across all slabs
    double t_max = ray_t.m_max;

    std::size_t hit_axis = 0;
    bool hit_max_face = false;

    for (std::size_t axis = 0; axis < 3; axis++)
    {
        const Interval& axis_interval = m_bounding_box[axis];
        const double inverse_ray_direction = 1.0 / ray.m_direction[axis];
        const bool ray_direction_is_negative = inverse_ray_direction < 0;

        // Negative direction hits max plane first, positive hits min first
        const double t_near = ((ray_direction_is_negative ? axis_interval.m_max : axis_interval.m_min) - ray.m_origin[axis]) * inverse_ray_direction;
        const double t_far  = ((ray_direction_is_negative ? axis_interval.m_min : axis_interval.m_max) - ray.m_origin[axis]) * inverse_ray_direction;

        if (t_near > t_min)
        {
            t_min = t_near;
            hit_axis = axis;
            hit_max_face = ray_direction_is_negative;
        }

        t_max = std::min(t_max, t_far);

        // Ray misses by exiting one slab before entering another
        if (t_min > t_max)
        {
            return false;
        }
    }

    if (!ray_t.Surrounds(t_min))
    {
        return false;
    }

    out_result.m_t = t_min;
    out_result.m_point = ray.At(t_min);

    Vec3 outward_normal(0.0);
    if (hit_max_face)
    {
        outward_normal[hit_axis] = 1.0;
    }
    else
    {
        outward_normal[hit_axis] = -1.0;
    }
    out_result.SetFaceNormal(ray, outward_normal);

    // UV: project onto the two axes perpendicular to hit face
    int u_axis = (hit_axis + 1) % 3;
    int v_axis = (hit_axis + 2) % 3;
    out_result.m_u = (out_result.m_point[u_axis] - m_bounding_box[u_axis].m_min) / m_bounding_box[u_axis].Size();
    out_result.m_v = (out_result.m_point[v_axis] - m_bounding_box[v_axis].m_min) / m_bounding_box[v_axis].Size();

    out_result.m_material = m_material;
    return true;
}

AABB AxisAlignedBox::BoundingBox() const
{
    return m_bounding_box;
}

} // namespace ART
