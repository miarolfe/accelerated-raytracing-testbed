// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include "Common.h"
#include "Ray.h"
#include "Vec3.h"

namespace ART
{

// Fwd decl
class Material;

// Results for a ray intersection test
struct RayHitResult
{
public:
    Point3 m_point;
    Vec3 m_normal;
    double m_t;
    double m_u;
    double m_v;
    std::shared_ptr<Material> m_material;
    bool m_is_front_facing;

    // Determine the correct face normal
    void SetFaceNormal(const Ray& ray, const Vec3& outward_normal);
};

} // namespace ART
