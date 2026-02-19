// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Acceleration/KDTree.h>
#include <Core/ArenaAllocator.h>
#include <Core/Constants.h>
#include <Geometry/Sphere.h>
#include <Materials/Material.h>

namespace ART
{

TEST_CASE("KDTreeNode constructor with vector of objects", "[KDTreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, material));

        KDTreeNode kd_tree(objects);
        const AABB box = kd_tree.BoundingBox();

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

        KDTreeNode kd_tree(objects);
        const AABB box = kd_tree.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(-1.5));
        REQUIRE(box.m_x.m_max == Approx(2.5));
        REQUIRE(box.m_y.m_min == Approx(-1.5));
        REQUIRE(box.m_y.m_max == Approx(1.5));
    }
}

TEST_CASE("KDTreeNode Hit detects intersections", "[KDTreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Ray hits single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        KDTreeNode kd_tree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = kd_tree.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
    }

    SECTION("Ray misses all objects")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3( 10.0, 0.0, -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(-10.0, 0.0, -5.0), 1.0, material));

        KDTreeNode kd_tree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = kd_tree.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == false);
    }

    SECTION("Ray hits closest object among multiple")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -10.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0,  -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0,  -3.0), 0.5, material));

        KDTreeNode kd_tree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = kd_tree.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
        REQUIRE(result.m_t == Approx(2.5));
    }

    SECTION("Ray respects interval bounds")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        KDTreeNode kd_tree(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;

        // Sphere at z = -5 with radius = 1, hit at t = 4, so interval starting at t = 10 must miss
        const bool hit = kd_tree.Hit(ray, Interval(10.0, infinity), result);
        REQUIRE(hit == false);
    }
}

TEST_CASE("KDTreeNode Hit uses SplitLongestAxis fallback when all centroids are in the same plane", "[KDTreeNode]")
{
    // Place all sphere centroids at the same point on every axis (within fp_tolerance, 1e-10)
    // SplitSAH will compute extent < 1e-10 for all 3 axes and skip them, falling back to SplitLongestAxis
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    for (int i = 0; i < 10; i++)
    {
        // All centroids within 1e-12 of (0, 0, -5) — well below the 1e-10 tolerance
        const double sub_tolerance_offset = static_cast<double>(i) * 1e-12;
        objects.push_back(allocator.Create<Sphere>(Point3(sub_tolerance_offset, 0.0, -5.0), 0.5, material));
    }

    KDTreeNode kd_tree(objects);

    // Ray aimed at the cluster must hit
    {
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = kd_tree.Hit(ray, Interval(0.001, infinity), result);
        REQUIRE(hit == true);
    }

    // Ray aimed away from the cluster must miss
    {
        const Ray ray(Point3(10.0, 10.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = kd_tree.Hit(ray, Interval(0.001, infinity), result);
        REQUIRE(hit == false);
    }
}

TEST_CASE("KDTreeNode Hit traverses in correct order for negative-direction ray (should_swap_child_order)", "[KDTreeNode]")
{
    // After SAH splits along an axis (x in this case, objects are spread along x)
    // ray going -x satisfies (ray_direction_along_axis < 0) so should_swap_order = true
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    // Spread along x so SAH picks an x-axis split
    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3(-4.0, 0.0, 0.0), 0.5, material));
    objects.push_back(allocator.Create<Sphere>(Point3(-2.0, 0.0, 0.0), 0.5, material));
    objects.push_back(allocator.Create<Sphere>(Point3( 2.0, 0.0, 0.0), 0.5, material));
    objects.push_back(allocator.Create<Sphere>(Point3( 4.0, 0.0, 0.0), 0.5, material));

    KDTreeNode kd_tree(objects);

    // Ray from +x going -x hits the rightmost sphere first
    const Ray ray(Point3(10.0, 0.0, 0.0), Vec3(-1.0, 0.0, 0.0));
    RayHitResult result;
    const bool hit = kd_tree.Hit(ray, Interval(0.001, infinity), result);

    REQUIRE(hit == true);
    REQUIRE(result.m_t == Approx(5.5)); // t = 10.0 - 4.5 = 5.5
}

TEST_CASE("KDTreeNode MemoryUsedBytes is non-zero for a non-trivial tree", "[KDTreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    for (int i = 0; i < 10; i++)
    {
        objects.push_back(allocator.Create<Sphere>(Point3(static_cast<double>(i), 0.0, -5.0), 0.5, material));
    }

    KDTreeNode kd_tree(objects);

    REQUIRE(kd_tree.MemoryUsedBytes() > 0);
}

TEST_CASE("KDTreeNode BoundingBox encloses all objects", "[KDTreeNode]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3(-5.0, -5.0, -5.0), 1.0, material));
    objects.push_back(allocator.Create<Sphere>(Point3( 5.0,  5.0,  5.0), 1.0, material));

    KDTreeNode kd_tree(objects);
    const AABB box = kd_tree.BoundingBox();

    REQUIRE(box.m_x.m_min <= -6.0);
    REQUIRE(box.m_x.m_max >= 6.0);
    REQUIRE(box.m_y.m_min <= -6.0);
    REQUIRE(box.m_y.m_max >= 6.0);
    REQUIRE(box.m_z.m_min <= -6.0);
    REQUIRE(box.m_z.m_max >= 6.0);
}

} // namespace ART
