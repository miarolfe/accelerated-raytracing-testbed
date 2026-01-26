// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Acceleration/BoundingVolumeHierarchy.h>
#include <Core/ArenaAllocator.h>
#include <Core/Constants.h>
#include <Geometry/Sphere.h>
#include <Materials/Material.h>

namespace ART
{

TEST_CASE("BVHNode constructs from vector of objects", "[BVHNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.7));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, material));

        BVHNode bounding_volume_hierarchy(objects);
        const AABB box = bounding_volume_hierarchy.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(-0.5));
        REQUIRE(box.m_x.m_max == Approx(0.5));
        REQUIRE(box.m_y.m_min == Approx(-0.5));
        REQUIRE(box.m_y.m_max == Approx(0.5));
        REQUIRE(box.m_z.m_min == Approx(-1.5));
        REQUIRE(box.m_z.m_max == Approx(-0.5));
    }

    SECTION("Two objects")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3(2.0, 0.0, -1.0), 0.5, material));

        BVHNode bounding_volume_hierarchy(objects);
        const AABB box = bounding_volume_hierarchy.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(-0.5));
        REQUIRE(box.m_x.m_max == Approx(2.5));
    }

    SECTION("Multiple objects")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3(2.0, 0.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3(1.0, 1.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3(-1.0, -1.0, -1.0), 0.5, material));

        BVHNode bounding_volume_hierarchy(objects);
        const AABB box = bounding_volume_hierarchy.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(-1.5));
        REQUIRE(box.m_x.m_max == Approx(2.5));
        REQUIRE(box.m_y.m_min == Approx(-1.5));
        REQUIRE(box.m_y.m_max == Approx(1.5));
    }
}

TEST_CASE("BVHNode Hit detects intersections", "[BVHNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.7));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));

    SECTION("Ray hits single object in BVH")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        BVHNode bounding_volume_hierarchy(objects);
        RayHitResult result;
        const bool hit = bounding_volume_hierarchy.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
        REQUIRE(result.m_t > 0.0);
    }

    SECTION("Ray misses all objects in BVH")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(10.0, 0.0, -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(-10.0, 0.0, -5.0), 1.0, material));

        BVHNode bounding_volume_hierarchy(objects);
        RayHitResult result;
        const bool hit = bounding_volume_hierarchy.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == false);
    }

    SECTION("Ray hits closest object among multiple")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -10.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -3.0), 0.5, material));

        BVHNode bounding_volume_hierarchy(objects);
        RayHitResult result;
        const bool hit = bounding_volume_hierarchy.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
        REQUIRE(result.m_t == Approx(2.5));
    }

    SECTION("Ray respects interval bounds")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        BVHNode bounding_volume_hierarchy(objects);
        RayHitResult result;

        const bool hit = bounding_volume_hierarchy.Hit(ray, Interval(10.0, infinity), result);
        REQUIRE(hit == false);
    }
}

TEST_CASE("BVHNode constructs correct tree structure", "[BVHNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.7));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Many objects create hierarchical structure")
    {
        std::vector<IRayHittable*> objects;

        for (std::size_t object_index = 0; object_index < 10; object_index++)
        {
            objects.push_back
            (
                allocator.Create<Sphere>
                (
                    Point3(static_cast<double>(object_index), 0.0, -5.0),
                    0.5,
                    material
                )
            );
        }

        BVHNode bounding_volume_hierarchy(objects);

        Ray ray(Point3(5.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = bounding_volume_hierarchy.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
    }
}

TEST_CASE("BVHNode BoundingBox encloses all objects", "[BVHNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.7));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Bounding box contains all spheres")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(-5.0, -5.0, -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(5.0, 5.0, 5.0), 1.0, material));

        BVHNode bounding_volume_hierarchy(objects);
        const AABB box = bounding_volume_hierarchy.BoundingBox();

        REQUIRE(box.m_x.m_min <= -6.0);
        REQUIRE(box.m_x.m_max >= 6.0);
        REQUIRE(box.m_y.m_min <= -6.0);
        REQUIRE(box.m_y.m_max >= 6.0);
        REQUIRE(box.m_z.m_min <= -6.0);
        REQUIRE(box.m_z.m_max >= 6.0);
    }
}

} // namespace ART
