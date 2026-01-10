// Copyright Mia Rolfe. All rights reserved.
#include "BoundingVolumeHierarchy.h"

namespace ART
{

BoundingVolumeHierarchy::BoundingVolumeHierarchy(std::vector<IRayHittable*>& objects)
{

}

BoundingVolumeHierarchy::~BoundingVolumeHierarchy()
{

}

bool BoundingVolumeHierarchy::Hit(const Ray&, Interval ray_t, RayHitResult& out_result) const
{
    // TODO
    return false;
}

AABB BoundingVolumeHierarchy::BoundingBox() const
{
    return m_bounding_box;
}

void BoundingVolumeHierarchy::Create(std::vector<IRayHittable*>& objects)
{

}

void BoundingVolumeHierarchy::Destroy()
{

}

} // namespace ART
