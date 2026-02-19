// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Acceleration/UniformGrid.h>
#include <Core/ArenaAllocator.h>
#include <Core/Constants.h>
#include <Geometry/Sphere.h>
#include <Materials/Material.h>

namespace ART
{

TEST_CASE("UniformGrid constructor with vector of objects", "[UniformGrid]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, material));

        UniformGrid grid(objects);
        const AABB box = grid.BoundingBox();

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
        objects.push_back(allocator.Create<Sphere>(Point3(0.0,  0.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3(2.0,  0.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3(1.0,  1.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3(-1.0, -1.0, -1.0), 0.5, material));

        UniformGrid grid(objects);
        const AABB box = grid.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(-1.5));
        REQUIRE(box.m_x.m_max == Approx(2.5));
        REQUIRE(box.m_y.m_min == Approx(-1.5));
        REQUIRE(box.m_y.m_max == Approx(1.5));
    }
}

TEST_CASE("UniformGrid Hit detects intersections", "[UniformGrid]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Ray hits single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        UniformGrid grid(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = grid.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
    }

    SECTION("Ray misses all objects")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(10.0, 0.0, -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(-10.0, 0.0, -5.0), 1.0, material));

        UniformGrid grid(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = grid.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == false);
    }

    SECTION("Ray hits closest object among multiple")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -10.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0),  1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -3.0),  0.5, material));

        UniformGrid grid(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = grid.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
        REQUIRE(result.m_t == Approx(2.5));
    }

    SECTION("Ray respects interval bounds")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        UniformGrid grid(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;

        // Sphere is at z = -5 with radius = 1, so hit at t=4; starting interval at t=10 must miss
        const bool hit = grid.Hit(ray, Interval(10.0, infinity), result);
        REQUIRE(hit == false);
    }
}

TEST_CASE("UniformGrid Hit detects negative-direction ray (step_x = -1)", "[UniformGrid]")
{
    // Tests 3DDDA negative-step branch: ray travelling in the -x direction
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, 0.0), 1.0, material));

    UniformGrid grid(objects);

    // Ray from +x going in -x direction; hits sphere at t = 4 (surface at x = 1, origin at x = 5)
    const Ray ray(Point3(5.0, 0.0, 0.0), Vec3(-1.0, 0.0, 0.0));
    RayHitResult result;
    const bool hit = grid.Hit(ray, Interval(0.001, infinity), result);

    REQUIRE(hit == true);
    REQUIRE(result.m_t == Approx(4.0));
}

TEST_CASE("UniformGrid Hit detects large sphere spanning multiple cells", "[UniformGrid]")
{
    // A sphere large enough to span multiple grid cells must register in all of them
    // Any ray aimed at sphere's centre must still produce a hit regardless of which cells the ray traverses
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    // Large sphere plus several small ones to ensure a reasonable cell size
    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -10.0), 5.0, material));
    objects.push_back(allocator.Create<Sphere>(Point3(-8.0, 0.0, 0.0),  0.5, material));
    objects.push_back(allocator.Create<Sphere>(Point3( 8.0, 0.0, 0.0),  0.5, material));
    objects.push_back(allocator.Create<Sphere>(Point3( 0.0, 8.0, 0.0),  0.5, material));

    UniformGrid grid(objects);

    // Ray aimed at the centre of the large sphere from several origins
    // Each passes through different cell columns but must still hit.
    const Point3 ray_origins[] =
    {
        Point3( 0.0, 0.0, 0.0),
        Point3( 2.0, 0.0, 0.0),
        Point3(-2.0, 0.0, 0.0),
    };

    for (const Point3& ray_origin : ray_origins)
    {
        const Ray ray(ray_origin, Normalised(Vec3(0.0, 0.0, -1.0) + Vec3(ray_origin.m_x * -0.05, 0.0, 0.0)));
        RayHitResult result;
        const bool hit = grid.Hit(ray, Interval(0.001, infinity), result);
        REQUIRE(hit == true);
    }
}

TEST_CASE("UniformGrid Hit works with a single-object scene", "[UniformGrid]")
{
    // With only 1 object, DetermineCellSize produces a cell size of 3*max_extent,
    // so the whole scene fits in single 1x1x1 grid (one cell).
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -3.0), 1.0, material));

    UniformGrid grid(objects);

    SECTION("Ray hits the single object")
    {
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = grid.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
        REQUIRE(result.m_t == Approx(2.0));
    }

    SECTION("Ray misses the single object")
    {
        const Ray ray(Point3(5.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = grid.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == false);
    }
}

TEST_CASE("UniformGrid MemoryUsedBytes is non-zero for a non-trivial scene", "[UniformGrid]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    for (int i = 0; i < 10; i++)
    {
        objects.push_back(allocator.Create<Sphere>(Point3(static_cast<double>(i), 0.0, -5.0), 0.5, material));
    }

    UniformGrid grid(objects);

    REQUIRE(grid.MemoryUsedBytes() > 0);
}

TEST_CASE("UniformGrid BoundingBox encloses all objects in grid", "[UniformGrid]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3(-5.0, -5.0, -5.0), 1.0, material));
    objects.push_back(allocator.Create<Sphere>(Point3( 5.0,  5.0,  5.0), 1.0, material));

    UniformGrid grid(objects);
    const AABB box = grid.BoundingBox();

    REQUIRE(box.m_x.m_min <= -6.0);
    REQUIRE(box.m_x.m_max >= 6.0);
    REQUIRE(box.m_y.m_min <= -6.0);
    REQUIRE(box.m_y.m_max >= 6.0);
    REQUIRE(box.m_z.m_min <= -6.0);
    REQUIRE(box.m_z.m_max >= 6.0);
}

} // namespace ART
