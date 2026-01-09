// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <ArenaAllocator.h>
#include <Material.h>
#include <Sphere.h>
#include <Ray.h>
#include <Vec3.h>

namespace ART
{

TEST_CASE("Sphere constructors initialize correctly", "[Sphere]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.7));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Default constructor")
    {
        Sphere sphere;
        REQUIRE(sphere.m_centre.m_origin.m_x == Approx(0.0));
        REQUIRE(sphere.m_centre.m_origin.m_y == Approx(0.0));
        REQUIRE(sphere.m_centre.m_origin.m_z == Approx(0.0));
        REQUIRE(sphere.m_radius == Approx(1.0));
    }

    SECTION("Stationary sphere constructor")
    {
        Point3 centre(1.0, 2.0, 3.0);
        double radius = 2.5;
        Sphere sphere(centre, radius, material);

        REQUIRE(sphere.m_centre.m_origin.m_x == Approx(1.0));
        REQUIRE(sphere.m_centre.m_origin.m_y == Approx(2.0));
        REQUIRE(sphere.m_centre.m_origin.m_z == Approx(3.0));
        REQUIRE(sphere.m_radius == Approx(2.5));
    }

    SECTION("Moving sphere constructor")
    {
        Point3 start(0.0, 0.0, 0.0);
        Point3 end(1.0, 1.0, 1.0);
        double radius = 1.0;
        Sphere sphere(start, end, radius, material);

        REQUIRE(sphere.m_centre.m_origin.m_x == Approx(0.0));
        REQUIRE(sphere.m_centre.m_origin.m_y == Approx(0.0));
        REQUIRE(sphere.m_centre.m_origin.m_z == Approx(0.0));
        REQUIRE(sphere.m_centre.m_direction.m_x == Approx(1.0));
        REQUIRE(sphere.m_centre.m_direction.m_y == Approx(1.0));
        REQUIRE(sphere.m_centre.m_direction.m_z == Approx(1.0));
        REQUIRE(sphere.m_radius == Approx(1.0));
    }
}

TEST_CASE("Sphere Hit detects intersections correctly", "[Sphere]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.7));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    Sphere sphere(Point3(0, 0.0, -5.0), 1.0, material);
    Interval t_range(0.001, 1000.0);

    SECTION("Ray hits front of sphere")
    {
        Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;

        bool hit = sphere.Hit(ray, t_range, result);
        REQUIRE(hit == true);
        REQUIRE(result.m_is_front_facing == true);
        REQUIRE(result.m_t > 0.0);
        REQUIRE(result.m_point.m_z < 0.0);
    }

    SECTION("Ray misses sphere")
    {
        Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 1.0, 0.0));
        RayHitResult result;

        bool hit = sphere.Hit(ray, t_range, result);
        REQUIRE(hit == false);
    }

    SECTION("Ray hits back of sphere")
    {
        Ray ray(Point3(0.0, 0.0, -5.0), Vec3(0.0, 0.0, 1.0));
        RayHitResult result;

        bool hit = sphere.Hit(ray, t_range, result);
        REQUIRE(hit == true);
        REQUIRE(result.m_is_front_facing == false);
    }
}

TEST_CASE("Sphere GetUVOnUnitSphere returns valid coordinates", "[Sphere]")
{
    double u, v;

    SECTION("Point at north pole")
    {
        Sphere::GetUVOnUnitSphere(Point3(0.0, 1.0, 0.0), u, v);
        REQUIRE(u >= 0.0);
        REQUIRE(u <= 1.0);
        REQUIRE(v == Approx(0.0));
    }

    SECTION("Point at south pole")
    {
        Sphere::GetUVOnUnitSphere(Point3(0.0, -1.0, 0.0), u, v);
        REQUIRE(u >= 0.0);
        REQUIRE(u <= 1.0);
        REQUIRE(v == Approx(1.0));
    }

    SECTION("Point on equator")
    {
        Sphere::GetUVOnUnitSphere(Point3(1.0, 0.0, 0.0), u, v);
        REQUIRE(v == Approx(0.5));
        REQUIRE(u >= 0.0);
        REQUIRE(u <= 1.0);
    }
}

TEST_CASE("Sphere BoundingBox returns expected box", "[Sphere]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.7));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    Sphere sphere(Point3(0.0, 0.0, 0.0), 1.0, material);
    AABB aabb = sphere.BoundingBox();

    REQUIRE(aabb.m_x.m_min == Approx(-1.0));
    REQUIRE(aabb.m_y.m_min == Approx(-1.0));
    REQUIRE(aabb.m_z.m_min == Approx(-1.0));

    REQUIRE(aabb.m_x.m_max == Approx(1.0));
    REQUIRE(aabb.m_y.m_max == Approx(1.0));
    REQUIRE(aabb.m_z.m_max == Approx(1.0));
}

} // namespace ART
