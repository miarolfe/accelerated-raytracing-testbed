// Copyright Mia Rolfe. All rights reserved.
#include "Sphere.h"
#include "AxisAlignedBoundingBox.h"
#include "Material.h"

namespace ART
{

Sphere::Sphere()
{
    m_centre = Ray(Point3(0.0), Vec3(0.0));
    m_radius = 1.0;
    const Vec3 radius_vec = Vec3(m_radius);
	m_bounding_box = AABB(m_centre.m_origin - radius_vec, m_centre.m_origin + radius_vec);
	m_material = std::make_shared<LambertianMaterial>(Colour(0.7));
}

Sphere::Sphere(const Point3& centre, double radius, std::shared_ptr<Material> material)
{
    assert(radius >= 0.0);
    m_centre = Ray(centre, Vec3(0.0));
    m_radius = radius;
    const Vec3 radius_vec = Vec3(m_radius);
   	m_bounding_box = AABB(m_centre.m_origin - radius_vec, m_centre.m_origin + radius_vec);
    m_material = material;
}

Sphere::Sphere(const Point3& centre1, const Point3& centre2, double radius, std::shared_ptr<Material> material)
{
    assert(radius >= 0.0);
    m_centre = Ray(centre1, centre2 - centre1);
    m_radius = std::fmax(0.0, radius);
    const Vec3 radius_vec = Vec3(radius);
    AABB start_bounding_box(m_centre.At(0.0) - radius_vec, m_centre.At(0.0) + radius_vec);
    AABB end_bounding_box(m_centre.At(1.0) - radius_vec, m_centre.At(1.0) + radius_vec);
    m_bounding_box = AABB(start_bounding_box, end_bounding_box);
    m_material = material;
}

bool Sphere::Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const
{
    const Point3 current_centre = m_centre.At(ray.m_time);
    const Vec3 oc = current_centre - ray.m_origin;
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
    const Vec3 outward_facing_normal = (out_result.m_point - current_centre) / m_radius;
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
