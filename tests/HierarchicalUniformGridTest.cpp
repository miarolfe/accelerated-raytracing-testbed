// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Acceleration/HierarchicalUniformGrid.h>
#include <Acceleration/UniformGrid.h>
#include <Core/ArenaAllocator.h>
#include <Core/Constants.h>
#include <Geometry/Sphere.h>
#include <Materials/Material.h>

namespace ART
{

TEST_CASE("HierarchicalUniformGrid constructor with vector of objects", "[HierarchicalUniformGrid]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, material));

        HierarchicalUniformGrid grid(objects);
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
        objects.push_back(allocator.Create<Sphere>(Point3( 0.0,  0.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3( 2.0,  0.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3( 1.0,  1.0, -1.0), 0.5, material));
        objects.push_back(allocator.Create<Sphere>(Point3(-1.0, -1.0, -1.0), 0.5, material));

        HierarchicalUniformGrid grid(objects);
        const AABB box = grid.BoundingBox();

        REQUIRE(box.m_x.m_min == Approx(-1.5));
        REQUIRE(box.m_x.m_max == Approx(2.5));
        REQUIRE(box.m_y.m_min == Approx(-1.5));
        REQUIRE(box.m_y.m_max == Approx(1.5));
    }
}

TEST_CASE("HierarchicalUniformGrid Hit detects intersections", "[HierarchicalUniformGrid]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Ray hits single object")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -5.0), 1.0, material));

        HierarchicalUniformGrid grid(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = grid.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == true);
    }

    SECTION("Ray misses all objects")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3( 10.0, 0.0, -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(-10.0, 0.0, -5.0), 1.0, material));

        HierarchicalUniformGrid grid(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;
        const bool hit = grid.Hit(ray, Interval(0.001, infinity), result);

        REQUIRE(hit == false);
    }

    SECTION("Ray hits closest object among multiple")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -10.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0,  -5.0), 1.0, material));
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0,  -3.0), 0.5, material));

        HierarchicalUniformGrid grid(objects);
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

        HierarchicalUniformGrid grid(objects);
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, -1.0));
        RayHitResult result;

        // Sphere at z=-5 with r=1, hit at t=4; interval starting at t=10 must miss
        const bool hit = grid.Hit(ray, Interval(10.0, infinity), result);
        REQUIRE(hit == false);
    }
}

TEST_CASE("HierarchicalUniformGrid produces the same closest hit as UniformGrid", "[HierarchicalUniformGrid]")
{
    // Both structures use identical 3DDDA traversal logic
    // The same scene should give the same result.
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    // Use enough objects so both structures build multi-cell grids
    std::vector<IRayHittable*> objects_uniform_grid;
    objects_uniform_grid.push_back(allocator.Create<Sphere>(Point3( 0.0, 0.0, -10.0), 1.0, material));
    objects_uniform_grid.push_back(allocator.Create<Sphere>(Point3( 0.0, 0.0,  -5.0), 1.0, material));
    objects_uniform_grid.push_back(allocator.Create<Sphere>(Point3( 0.0, 0.0,  -3.0), 0.5, material));
    objects_uniform_grid.push_back(allocator.Create<Sphere>(Point3( 3.0, 0.0,  -5.0), 0.5, material));
    objects_uniform_grid.push_back(allocator.Create<Sphere>(Point3(-3.0, 0.0,  -5.0), 0.5, material));

    // Need separate copies of the vector because both constructors take non-const ref
    std::vector<IRayHittable*> objects_hierarchical_uniform_grid(objects_uniform_grid);

    UniformGrid uniform_grid(objects_uniform_grid);
    HierarchicalUniformGrid hierarchical_grid(objects_hierarchical_uniform_grid);

    const Ray rays[] =
    {
        Ray(Point3(0.0, 0.0, 0.0), Vec3( 0.0, 0.0, -1.0)),
        Ray(Point3(3.0, 0.0, 0.0), Vec3( 0.0, 0.0, -1.0)),
        Ray(Point3(5.0, 0.0, 0.0), Vec3( 0.0, 0.0, -1.0))
    };

    for (const Ray& ray : rays)
    {
        RayHitResult uniform_result;
        RayHitResult hierarchical_result;

        const bool did_ray_hit_uniform_grid = uniform_grid.Hit(ray, Interval(0.001, infinity), uniform_result);
        const bool did_ray_hit_hierarchical_uniform_grid = hierarchical_grid.Hit(ray, Interval(0.001, infinity), hierarchical_result);

        REQUIRE(did_ray_hit_uniform_grid == did_ray_hit_hierarchical_uniform_grid);
        if (did_ray_hit_uniform_grid)
        {
            REQUIRE(hierarchical_result.m_t == Approx(uniform_result.m_t));
        }
    }
}

TEST_CASE("HierarchicalUniformGrid MemoryUsedBytes is non-zero (subgrid allocation)", "[HierarchicalUniformGrid]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    for (int i = 0; i < 10; i++)
    {
        objects.push_back(allocator.Create<Sphere>(Point3(static_cast<double>(i), 0.0, -5.0), 0.5, material));
    }

    HierarchicalUniformGrid grid(objects);

    REQUIRE(grid.MemoryUsedBytes() > 0);
}

TEST_CASE("HierarchicalUniformGrid destructor does not crash (subgrid cleanup)", "[HierarchicalUniformGrid]")
{
    // Exercises Destroy() path that iterates all cells and deletes non-null subgrids
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    SECTION("Non-trivial scene")
    {
        std::vector<IRayHittable*> objects;
        for (int i = 0; i < 20; i++)
        {
            const int i_divided_by_five = i / 5;
            objects.push_back(allocator.Create<Sphere>
            (
                Point3(static_cast<double>(i % 5), static_cast<double>(i_divided_by_five), -5.0),
                0.4,
                material
            ));
        }

        // Constructor + destructor via scope exit
        {
            HierarchicalUniformGrid grid(objects);
        }
        REQUIRE(true); // Reached without crashing
    }

    SECTION("Single-object scene (minimal subgrid)")
    {
        std::vector<IRayHittable*> objects;
        objects.push_back(allocator.Create<Sphere>(Point3(0.0, 0.0, -3.0), 1.0, material));

        // Constructor + destructor via scope exit
        {
            HierarchicalUniformGrid grid(objects);
        }
        REQUIRE(true);
    }
}

TEST_CASE("HierarchicalUniformGrid BoundingBox encloses all objects", "[HierarchicalUniformGrid]")
{
    ArenaAllocator allocator(ONE_MEGABYTE);
    Texture* texture = allocator.Create<SolidColourTexture>(Colour(0.5));
    Material* material = allocator.Create<LambertianMaterial>(texture);

    std::vector<IRayHittable*> objects;
    objects.push_back(allocator.Create<Sphere>(Point3(-5.0, -5.0, -5.0), 1.0, material));
    objects.push_back(allocator.Create<Sphere>(Point3( 5.0,  5.0,  5.0), 1.0, material));

    HierarchicalUniformGrid grid(objects);
    const AABB box = grid.BoundingBox();

    REQUIRE(box.m_x.m_min <= -6.0);
    REQUIRE(box.m_x.m_max >= 6.0);
    REQUIRE(box.m_y.m_min <= -6.0);
    REQUIRE(box.m_y.m_max >= 6.0);
    REQUIRE(box.m_z.m_min <= -6.0);
    REQUIRE(box.m_z.m_max >= 6.0);
}

} // namespace ART
