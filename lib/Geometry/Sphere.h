// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Geometry/AxisAlignedBoundingBox.h>
#include <Maths/Vec3.h>
#include <RayTracing/IRayHittable.h>

namespace ART
{

struct Sphere : IRayHittable
{
public:
    Point3 m_centre;
    double m_radius;
    AABB m_bounding_box;
    Material* m_material;

    Sphere(const Point3& centre, double radius, Material* material);

    // Check if a ray intersects this sphere
    // Returns the result details using out_result
    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    // Return the sphere's bounding box
    AABB BoundingBox() const override;

    // Gets UV coordinates in [0, 1] from a given point on the surface of
    // a unit sphere centred at (0, 0).
    // out_u and out_v are output parameters for the coordinates.
    static void GetUVOnUnitSphere(const Point3& point, double& out_u, double& out_v);
};

} // namespace ART
