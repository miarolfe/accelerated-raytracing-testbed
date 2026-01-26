// Copyright Mia Rolfe. All rights reserved.
#include <RayTracing/RayHitResult.h>

namespace ART
{

void RayHitResult::SetFaceNormal(const Ray& ray, const Vec3& outward_normal)
{
    m_is_front_facing = Dot(ray.m_direction, outward_normal) < 0.0;
	m_normal = m_is_front_facing ? outward_normal : -outward_normal;
}

} // namespace ART
