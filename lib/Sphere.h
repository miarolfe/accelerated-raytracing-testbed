// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include "AxisAlignedBoundingBox.h"
#include "Constants.h"
#include "IRayHittable.h"
#include "RayHitResult.h"
#include "Ray.h"
#include "Vec3.h"

namespace ART
{

struct Sphere : IRayHittable
{
public:
    // Using a ray to represent the centre of the sphere because
    // it can move over time. A static sphere is represented by a ray
    // of length 0.
    Ray m_centre;
    double m_radius;
    AABB m_bounding_box;
    std::shared_ptr<Material> m_material;

    // Default constructor
    Sphere();

    // Stationary sphere constructor
    Sphere(const Point3& centre, double radius, std::shared_ptr<Material> material);

    // Moving sphere constructor
    Sphere(const Point3& centre1, const Point3& centre2, double radius, std::shared_ptr<Material> material);

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
