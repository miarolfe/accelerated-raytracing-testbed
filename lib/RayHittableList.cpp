// Copyright Mia Rolfe. All rights reserved.

#include "RayHittableList.h"
#include "IRayHittable.h"
#include "RayHitResult.h"

namespace ART
{

RayHittableList::RayHittableList() {}

RayHittableList::RayHittableList(std::shared_ptr<IRayHittable> hittable)
{
    Add(hittable);
}

void RayHittableList::Clear()
{
    m_objects.clear();
}

void RayHittableList::Add(std::shared_ptr<IRayHittable> hittable)
{
    m_objects.push_back(hittable);
    m_bounding_box = AABB(m_bounding_box, hittable->BoundingBox());
}

bool RayHittableList::Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const
{
    RayHitResult temp_result;
    bool has_ray_hit_any_object = false;
    double closest_distance = ray_t.m_max;

    for (const std::shared_ptr<IRayHittable>& object : m_objects)
    {
        if (object->Hit(ray, Interval(ray_t.m_min, closest_distance), temp_result))
        {
            has_ray_hit_any_object = true;
            closest_distance = temp_result.m_t;
            out_result = temp_result;
        }
    }

    return has_ray_hit_any_object;
}

std::vector<std::shared_ptr<IRayHittable>>& RayHittableList::GetObjects()
{
    return m_objects;
}

AABB RayHittableList::BoundingBox() const
{
    return m_bounding_box;
}

} // namespace ART
