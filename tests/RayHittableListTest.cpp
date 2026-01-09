// Copyright Mia Rolfe. All rights reserved.
#include "../external/Catch2/catch.hpp"

#include "../lib/ArenaAllocator.h"
#include "../lib/Constants.h"
#include "../lib/Material.h"
#include "../lib/RayHittableList.h"
#include "../lib/Sphere.h"

namespace ART
{

TEST_CASE("RayHittableList basic behaviour (spheres only)", "[RayHittableList]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.7));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Default constructor creates empty list")
    {
        RayHittableList list;
        REQUIRE(list.GetObjects().empty());
    }

    SECTION("Add actually adds IRayHittables to the list")
    {
        RayHittableList list;

        Sphere* sphere = allocator.Create<Sphere>
        (
            Point3(0.0, 0.0, -1.0),
            0.5,
            material
        );

        list.Add(sphere);

        REQUIRE(list.GetObjects().size() == 1);
        REQUIRE(list.GetObjects()[0] == sphere);
    }

    SECTION("Clear removes all IRayHittables")
    {
        RayHittableList list;

        list.Add(allocator.Create<Sphere>
        (
            Point3(0.0, 0.0, -1.0),
            0.5,
            material
        ));

        list.Clear();

        REQUIRE(list.GetObjects().empty());
    }
}

TEST_CASE("RayHittableList Hit behaviour", "[RayHittableList]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.7));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));

    SECTION("Ray misses all IRayHittables")
    {
        RayHittableList list;
        list.Add(allocator.Create<Sphere>
        (
            Point3(0.0, 0.0, 5.0),
            0.5,
            material
        ));

        RayHitResult result;
        const bool hit = list.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == false);
    }

    SECTION("Single IRayHittable hit")
    {
        RayHittableList list;
        list.Add(allocator.Create<Sphere>
        (
            Point3(0.0, 0.0, -1.0),
            0.5,
            material
        ));

        RayHitResult result;
        const bool hit = list.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
        REQUIRE(result.m_t > 0.0);
    }

    SECTION("Closest IRayHittable hit is selected")
    {
        RayHittableList list;

        list.Add(allocator.Create<Sphere>
        (
            Point3(0.0, 0.0, -5.0),
            1.0,
            material
        ));
        list.Add(allocator.Create<Sphere>
        (
            Point3(0.0, 0.0, -2.0),
            0.5,
            material
        ));

        RayHitResult result;
        const bool hit = list.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
        REQUIRE(result.m_t == Approx(1.5));
    }
}

TEST_CASE("RayHittableList BoundingBox (spheres)", "[RayHittableList]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.7));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Empty list has default bounding box")
    {
        RayHittableList list;
        const AABB box = list.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(infinity));
        REQUIRE(box.m_x.m_max == Approx(-infinity));
    }

    SECTION("Bounding box encloses all spheres")
    {
        RayHittableList list;

        list.Add(allocator.Create<Sphere>
        (
            Point3(0.0, 0.0, 0.0),
            1.0,
            material
        ));
        list.Add(allocator.Create<Sphere>
        (
            Point3(3.0, 0.0, 0.0),
            2.0,
            material
        ));

        const AABB box = list.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(-1.0));
        REQUIRE(box.m_x.m_max == Approx(5.0));

        REQUIRE(box.m_y.m_min == Approx(-2.0));
        REQUIRE(box.m_y.m_max == Approx(2.0));

        REQUIRE(box.m_z.m_min == Approx(-2.0));
        REQUIRE(box.m_z.m_max == Approx(2.0));
    }
}

} // namespace ART
