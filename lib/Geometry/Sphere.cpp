// Copyright Mia Rolfe. All rights reserved.
#include <Geometry/Sphere.h>

#include <Geometry/AxisAlignedBoundingBox.h>
#include <Materials/Material.h>

namespace ART
{

Sphere::Sphere(const Point3& centre, double radius, Material* material)
    : m_centre(centre), m_radius(radius), m_material(material)
{
    assert(radius >= 0.0);
    const Vec3 radius_vec = Vec3(m_radius);
    m_bounding_box = AABB(m_centre - radius_vec, m_centre + radius_vec);
}

bool Sphere::Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const
{
    const Vec3 oc = m_centre - ray.m_origin;
    const double a = ray.m_direction.LengthSquared();
    const double h = Dot(ray.m_direction, oc);
    const double c = oc.LengthSquared() - (m_radius * m_radius);
    const double discriminant = (h * h) - (a * c);

    if (discriminant < 0)
    {
        return false;
    }

    const double square_root_of_discriminant = std::sqrt(discriminant);

    double root = (h - square_root_of_discriminant) / a;

    if (!ray_t.Surrounds(root))
    {
        root = (h + square_root_of_discriminant) / a;
    }

    if (!ray_t.Surrounds(root))
    {
        return false;
    }

    // Populate result
    out_result.m_t = root;
    out_result.m_point = ray.At(out_result.m_t);
    const Vec3 outward_facing_normal = (out_result.m_point - m_centre) / m_radius;
    out_result.SetFaceNormal(ray, outward_facing_normal);
    GetUVOnUnitSphere(outward_facing_normal, out_result.m_u, out_result.m_v);
    out_result.m_material = m_material;

    return true;
}

AABB Sphere::BoundingBox() const
{
    return m_bounding_box;
}

void Sphere::GetUVOnUnitSphere(const Point3& point, double& out_u, double& out_v)
{
    const double theta = std::acos(point.m_y);
    const double phi = std::atan2(-point.m_z, point.m_x) + pi;

    out_u = phi / (2.0 * pi);
    out_v = theta / pi;
}

} // namespace ART
