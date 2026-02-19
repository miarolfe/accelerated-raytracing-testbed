// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Geometry/AxisAlignedBox.h>
#include <Materials/Material.h>
#include <Materials/Texture.h>
#include <Maths/Ray.h>
#include <Core/Constants.h>

namespace ART
{

// Utility helper function.
// Unit box centred at origin: [-1, 1] on all axes
static AxisAlignedBox MakeUnitBox(Material* material)
{
    return AxisAlignedBox(Point3(-1.0), Point3(1.0), material);
}

TEST_CASE("AxisAlignedBox Hit detects ray from +z hitting front face", "[AxisAlignedBox]")
{
    SolidColourTexture texture(Colour(0.5));
    LambertianMaterial material(&texture);
    const AxisAlignedBox box = MakeUnitBox(&material);

    // Ray from (0, 0, 5) going in -z direction hits the +z face at t=4
    const Ray ray(Point3(0.0, 0.0, 5.0), Vec3(0.0, 0.0, -1.0));
    RayHitResult result;
    const bool hit = box.Hit(ray, Interval(0.001, infinity), result);

    REQUIRE(hit == true);
    REQUIRE(result.m_t == Approx(4.0));
    REQUIRE(result.m_material == &material);
    REQUIRE(result.m_is_front_facing == true);
}

TEST_CASE("AxisAlignedBox Hit detects ray from -x hitting left face", "[AxisAlignedBox]")
{
    SolidColourTexture texture(Colour(0.5));
    LambertianMaterial material(&texture);
    const AxisAlignedBox box = MakeUnitBox(&material);

    // Ray from (-5, 0, 0) going in +x direction hits the -x face at t=4
    const Ray ray(Point3(-5.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0));
    RayHitResult result;
    const bool hit = box.Hit(ray, Interval(0.001, infinity), result);

    REQUIRE(hit == true);
    REQUIRE(result.m_t == Approx(4.0));
    REQUIRE(result.m_is_front_facing == true);
}

TEST_CASE("AxisAlignedBox Hit misses a ray that does not intersect the box", "[AxisAlignedBox]")
{
    SolidColourTexture texture(Colour(0.5));
    LambertianMaterial material(&texture);
    const AxisAlignedBox box = MakeUnitBox(&material);

    // Ray going in +y direction from below offset misses entirely
    const Ray ray(Point3(10.0, -5.0, 0.0), Vec3(0.0, 1.0, 0.0));
    RayHitResult result;
    const bool hit = box.Hit(ray, Interval(0.001, infinity), result);

    REQUIRE(hit == false);
}

TEST_CASE("AxisAlignedBox Hit returns false for ray origin inside the box", "[AxisAlignedBox]")
{
    // Interior intersections are not supported by AxisAlignedBox.

    SolidColourTexture texture(Colour(0.5));
    LambertianMaterial material(&texture);
    const AxisAlignedBox box = MakeUnitBox(&material);

    // Ray origin inside the box going in +z
    const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0));
    RayHitResult result;
    const bool hit = box.Hit(ray, Interval(0.001, infinity), result);

    REQUIRE(hit == false);
}

TEST_CASE("AxisAlignedBox Hit produces UV coordinates in [0, 1]", "[AxisAlignedBox]")
{
    SolidColourTexture texture(Colour(0.5));
    LambertianMaterial material(&texture);
    const AxisAlignedBox box = MakeUnitBox(&material);

    // Several rays hitting different faces
    const Ray rays[] =
    {
        Ray(Point3(0.0,  0.0,  5.0), Vec3( 0.0,  0.0, -1.0)), // +z face
        Ray(Point3(0.0,  0.0, -5.0), Vec3( 0.0,  0.0,  1.0)), // -z face
        Ray(Point3(5.0,  0.0,  0.0), Vec3(-1.0,  0.0,  0.0)), // +x face
        Ray(Point3(0.0,  5.0,  0.0), Vec3( 0.0, -1.0,  0.0)), // +y face
    };

    for (const Ray& ray : rays)
    {
        RayHitResult result;
        const bool hit = box.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
        REQUIRE(result.m_u >= 0.0);
        REQUIRE(result.m_u <= 1.0);
        REQUIRE(result.m_v >= 0.0);
        REQUIRE(result.m_v <= 1.0);
    }
}

TEST_CASE("AxisAlignedBox Hit respects interval bounds", "[AxisAlignedBox]")
{
    SolidColourTexture texture(Colour(0.5));
    LambertianMaterial material(&texture);
    const AxisAlignedBox box = MakeUnitBox(&material);

    const Ray ray(Point3(0.0, 0.0, 5.0), Vec3(0.0, 0.0, -1.0));
    RayHitResult result;

    // Hit is at t=4; interval starting at t=10 should miss
    const bool hit = box.Hit(ray, Interval(10.0, infinity), result);

    REQUIRE(hit == false);
}

TEST_CASE("AxisAlignedBox BoundingBox returns correct min and max (extents)", "[AxisAlignedBox]")
{
    SolidColourTexture texture(Colour(0.5));
    LambertianMaterial material(&texture);
    const AxisAlignedBox box(Point3(-2.0, -3.0, -4.0), Point3(2.0, 3.0, 4.0), &material);

    const AABB aabb = box.BoundingBox();

    REQUIRE(aabb.m_x.m_min == Approx(-2.0));
    REQUIRE(aabb.m_x.m_max == Approx(2.0));
    REQUIRE(aabb.m_y.m_min == Approx(-3.0));
    REQUIRE(aabb.m_y.m_max == Approx(3.0));
    REQUIRE(aabb.m_z.m_min == Approx(-4.0));
    REQUIRE(aabb.m_z.m_max == Approx(4.0));
}

TEST_CASE("AxisAlignedBox BoundingBox constructor from AABB matches min/max constructor", "[AxisAlignedBox]")
{
    SolidColourTexture texture(Colour(0.5));
    LambertianMaterial material(&texture);

    const AABB aabb(Point3(-1.0, -1.0, -1.0), Point3(1.0, 1.0, 1.0));
    const AxisAlignedBox box_from_aabb(aabb, &material);
    const AxisAlignedBox box_from_min_and_max_points(Point3(-1.0, -1.0, -1.0), Point3(1.0, 1.0, 1.0), &material);

    const AABB result_aabb = box_from_aabb.BoundingBox();
    const AABB result_min_and_max_points  = box_from_min_and_max_points.BoundingBox();

    REQUIRE(result_aabb.m_x.m_min == Approx(result_min_and_max_points.m_x.m_min));
    REQUIRE(result_aabb.m_x.m_max == Approx(result_min_and_max_points.m_x.m_max));
    REQUIRE(result_aabb.m_z.m_min == Approx(result_min_and_max_points.m_z.m_min));
    REQUIRE(result_aabb.m_z.m_max == Approx(result_min_and_max_points.m_z.m_max));
}

} // namespace ART
