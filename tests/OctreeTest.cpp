// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Acceleration/Octree.h>
#include <Core/ArenaAllocator.h>
#include <Core/Constants.h>
#include <Geometry/Sphere.h>
#include <Materials/Material.h>

namespace ART
{

TEST_CASE("OctreeNode constructor with vector of objects", "[OctreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, material));

        OctreeNode octree(objects);
        const AABB box = octree.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(-0.5));
        REQUIRE(box.m_x.m_max == Approx(0.5));
        REQUIRE(box.m_y.m_min == Approx(-0.5));
        REQUIRE(box.m_y.m_max == Approx(0.5));
        REQUIRE(box.m_z.m_min == Approx(-1.5));
        REQUIRE(box.m_z.m_max == Approx(-0.5));
    }

    SECTION("Multiple objects")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3( 0.0,  0.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3( 2.0,  0.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3( 1.0,  1.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3(-1.0, -1.0, -1.0), 0.5, material));

        OctreeNode octree(objects);
        const AABB box = octree.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(-1.5));
        REQUIRE(box.m_x.m_max == Approx(2.5));
        REQUIRE(box.m_y.m_min == Approx(-1.5));
        REQUIRE(box.m_y.m_max == Approx(1.5));
    }
}

TEST_CASE("OctreeNode Hit detects intersections", "[OctreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Ray hits single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        OctreeNode octree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = octree.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
    }

    SECTION("Ray misses all objects")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3( 10.0, 0.0, -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(-10.0, 0.0, -5.0), 1.0, material));

        OctreeNode octree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = octree.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == false);
    }

    SECTION("Ray hits closest object among multiple")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -10.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0,  -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0,  -3.0), 0.5, material));

        OctreeNode tree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = tree.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
        REQUIRE(result.m_t == Approx(2.5));
    }

    SECTION("Ray respects interval bounds")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        OctreeNode octree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;

        // Sphere at z=-5 with r=1, hit at t=4; interval starting at t=10 must miss
        const bool hit = octree.Hit(ray, Interval(10.0, infinity), result);
        REQUIRE(hit == false);
    }
}

TEST_CASE("OctreeNode Hit finds spheres in all 8 octants", "[OctreeNode]")
{
    // Place one sphere per octant around the origin
    // The split centre will be at the origin, so each sphere's centroid sits cleaning in octant

    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    const double distance_from_origin = 3.0;
    // Small enough that spheres are well-separated
    const double radius = 0.4;

    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3(-distance_from_origin, -distance_from_origin, -distance_from_origin), radius, material)); // octant 0
    objects.push_back(allocator.Create<Sphere>(Point3(+distance_from_origin, -distance_from_origin, -distance_from_origin), radius, material)); // octant 1
    objects.push_back(allocator.Create<Sphere>(Point3(-distance_from_origin, +distance_from_origin, -distance_from_origin), radius, material)); // octant 2
    objects.push_back(allocator.Create<Sphere>(Point3(+distance_from_origin, +distance_from_origin, -distance_from_origin), radius, material)); // octant 3
    objects.push_back(allocator.Create<Sphere>(Point3(-distance_from_origin, -distance_from_origin, +distance_from_origin), radius, material)); // octant 4
    objects.push_back(allocator.Create<Sphere>(Point3(+distance_from_origin, -distance_from_origin, +distance_from_origin), radius, material)); // octant 5
    objects.push_back(allocator.Create<Sphere>(Point3(-distance_from_origin, +distance_from_origin, +distance_from_origin), radius, material)); // octant 6
    objects.push_back(allocator.Create<Sphere>(Point3(+distance_from_origin, +distance_from_origin, +distance_from_origin), radius, material)); // octant 7

    OctreeNode octree(objects);

    const Ray rays[] =
    {
        Ray(Point3(-distance_from_origin, -distance_from_origin, -20.0), Vec3(0.0, 0.0,  1.0)), // towards octant 0
        Ray(Point3(+distance_from_origin, -distance_from_origin, -20.0), Vec3(0.0, 0.0,  1.0)), // towards octant 1
        Ray(Point3(-distance_from_origin, +distance_from_origin, -20.0), Vec3(0.0, 0.0,  1.0)), // towards octant 2
        Ray(Point3(+distance_from_origin, +distance_from_origin, -20.0), Vec3(0.0, 0.0,  1.0)), // towards octant 3
        Ray(Point3(-distance_from_origin, -distance_from_origin, +20.0), Vec3(0.0, 0.0, -1.0)), // towards octant 4
        Ray(Point3(+distance_from_origin, -distance_from_origin, +20.0), Vec3(0.0, 0.0, -1.0)), // towards octant 5
        Ray(Point3(-distance_from_origin, +distance_from_origin, +20.0), Vec3(0.0, 0.0, -1.0)), // towards octant 6
        Ray(Point3(+distance_from_origin, +distance_from_origin, +20.0), Vec3(0.0, 0.0, -1.0)), // towards octant 7
    };

    for (const Ray& ray : rays)
    {
        RayHitResult result;
        const bool hit = octree.Hit(ray, Interval(0.001, infinity), result);
        REQUIRE(hit == true);
    }
}

TEST_CASE("OctreeNode MemoryUsedBytes is non-zero for a non-trivial tree", "[OctreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    for (int i = 0; i < 10; i++)
    {
        objects.push_back(allocator.Create<Sphere>(Point3(static_cast<double>(i), 0.0, -5.0), 0.5, material));
    }

    OctreeNode octree(objects);

    REQUIRE(octree.MemoryUsedBytes() > 0);
}

TEST_CASE("OctreeNode BoundingBox encloses all objects", "[OctreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3(-5.0, -5.0, -5.0), 1.0, material));
    objects.push_back(allocator.Create<Sphere>(Point3( 5.0,  5.0,  5.0), 1.0, material));

    OctreeNode octree(objects);
    const AABB box = octree.BoundingBox();

    REQUIRE(box.m_x.m_min <= -6.0);
    REQUIRE(box.m_x.m_max >= 6.0);
    REQUIRE(box.m_y.m_min <= -6.0);
    REQUIRE(box.m_y.m_max >= 6.0);
    REQUIRE(box.m_z.m_min <= -6.0);
    REQUIRE(box.m_z.m_max >= 6.0);
}

} // namespace ART
