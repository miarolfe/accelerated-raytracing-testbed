// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Acceleration/BSPTree.h>
#include <Core/ArenaAllocator.h>
#include <Core/Constants.h>
#include <Geometry/Sphere.h>
#include <Materials/Material.h>

namespace ART
{

TEST_CASE("BSPTreeNode constructor with vector of objects", "[BSPTreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, material));

        BSPTreeNode bsp_tree(objects);
        const AABB box = bsp_tree.BoundingBox();

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

        BSPTreeNode bsp_tree(objects);
        const AABB box = bsp_tree.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(-1.5));
        REQUIRE(box.m_x.m_max == Approx(2.5));
        REQUIRE(box.m_y.m_min == Approx(-1.5));
        REQUIRE(box.m_y.m_max == Approx(1.5));
    }
}

TEST_CASE("BSPTreeNode Hit detects intersections", "[BSPTreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Ray hits single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        BSPTreeNode bsp_tree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = bsp_tree.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
    }

    SECTION("Ray misses all objects")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3( 10.0, 0.0, -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(-10.0, 0.0, -5.0), 1.0, material));

        BSPTreeNode bsp_tree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = bsp_tree.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == false);
    }

    SECTION("Ray hits closest object among multiple")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -10.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0,  -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0,  -3.0), 0.5, material));

        BSPTreeNode bsp_tree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = bsp_tree.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
        REQUIRE(result.m_t == Approx(2.5));
    }

    SECTION("Ray respects interval bounds")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        BSPTreeNode bsp_tree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;

        // Sphere at z = -5 with radius = 1, hit at t = 4, interval starting at t = 10 must miss
        const bool hit = bsp_tree.Hit(ray, Interval(10.0, infinity), result);
        REQUIRE(hit == false);
    }
}

TEST_CASE("BSPTreeNode Hit finds spanning object from both sides of the split plane", "[BSPTreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    // The spanning sphere straddles x=0, add enough well-separated spheres so
    // FindSplitPlane can find a worthwhile x-axis split near x = 0
    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3( 0.0, 0.0, 0.0), 1.5, material)); // straddles x = 0, y = 0, and z = 0
    objects.push_back(allocator.Create<Sphere>(Point3( 6.0, 0.0, 0.0), 0.5, material)); // front (+x)
    objects.push_back(allocator.Create<Sphere>(Point3(-6.0, 0.0, 0.0), 0.5, material)); // back (-x)
    objects.push_back(allocator.Create<Sphere>(Point3( 6.0, 6.0, 0.0), 0.5, material)); // front (+x)
    objects.push_back(allocator.Create<Sphere>(Point3(-6.0, 6.0, 0.0), 0.5, material)); // back (-x)

    BSPTreeNode bsp_tree(objects);

    // Ray from +x side going through the spanning sphere
    {
        const Ray ray(Point3(10.0, 0.0, 0.0), Vec3(-1.0, 0.0, 0.0));
        RayHitResult result;
        const bool hit = bsp_tree.Hit(ray, Interval(0.001, infinity), result);
        REQUIRE(hit == true);
    }

    // Ray from -x side going through the spanning sphere
    {
        const Ray ray(Point3(-10.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0));
        RayHitResult result;
        const bool hit = bsp_tree.Hit(ray, Interval(0.001, infinity), result);
        REQUIRE(hit == true);
    }
}

TEST_CASE("BSPTreeNode Hit works when all objects are on the same side (index-split fallback)", "[BSPTreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    for (int i = 0; i < 10; i++)
    {
        // Offset by epsilon only — all centroids are essentially at (0, 0, -5)
        const double tiny_offset = static_cast<double>(i) * 1e-10;
        objects.push_back(allocator.Create<Sphere>(Point3(tiny_offset, 0.0, -5.0), 0.5, material));
    }

    BSPTreeNode bsp_tree(objects);

    // Ray aimed at cluster must hit
    {
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = bsp_tree.Hit(ray, Interval(0.001, infinity), result);
        REQUIRE(hit == true);
    }

    // Ray aimed away from cluster must miss
    {
        const Ray ray(Point3(10.0, 10.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = bsp_tree.Hit(ray, Interval(0.001, infinity), result);
        REQUIRE(hit == false);
    }
}

TEST_CASE("BSPTreeNode BoundingBox encloses all objects", "[BSPTreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3(-5.0, -5.0, -5.0), 1.0, material));
    objects.push_back(allocator.Create<Sphere>(Point3( 5.0,  5.0,  5.0), 1.0, material));

    BSPTreeNode bsp_tree(objects);
    const AABB box = bsp_tree.BoundingBox();

    REQUIRE(box.m_x.m_min <= -6.0);
    REQUIRE(box.m_x.m_max >= 6.0);
    REQUIRE(box.m_y.m_min <= -6.0);
    REQUIRE(box.m_y.m_max >= 6.0);
    REQUIRE(box.m_z.m_min <= -6.0);
    REQUIRE(box.m_z.m_max >= 6.0);
}

} // namespace ART
