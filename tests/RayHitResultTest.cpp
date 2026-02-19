// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Maths/Ray.h>
#include <Maths/Vec3.h>
#include <RayTracing/RayHitResult.h>

namespace ART
{

TEST_CASE("RayHitResult SetFaceNormal works correctly", "[RayHitResult]")
{
    SECTION("Front-facing ray")
    {
        Ray ray(Point3(0,0,0), Vec3(0,0,-1));
        Vec3 outward_normal(0,0,1);
        RayHitResult hit_result;
        hit_result.SetFaceNormal(ray, outward_normal);

        REQUIRE(hit_result.m_is_front_facing == true);
        REQUIRE(hit_result.m_normal.m_x == Approx(outward_normal.m_x));
        REQUIRE(hit_result.m_normal.m_y == Approx(outward_normal.m_y));
        REQUIRE(hit_result.m_normal.m_z == Approx(outward_normal.m_z));
    }

    SECTION("Back-facing ray")
    {
        Ray ray(Point3(0,0,0), Vec3(0,0,1));
        Vec3 outward_normal(0,0,1);
        RayHitResult hit_result;
        hit_result.SetFaceNormal(ray, outward_normal);

        REQUIRE(hit_result.m_is_front_facing == false);
        REQUIRE(hit_result.m_normal.m_x == Approx(0.0));
        REQUIRE(hit_result.m_normal.m_y == Approx(0.0));
        REQUIRE(hit_result.m_normal.m_z == Approx(-1.0));
    }
}

TEST_CASE("RayHitResult SetFaceNormal with a 45-degree incoming ray", "[RayHitResult]")
{
    SECTION("Ray approaching at 45 degrees to surface, front-facing")
    {
        // Ray direction: diagonal down-left, outward normal: +z
        // dot((1, 0, -1), (0, 0, 1)) = -1 < 0 -> front-facing, normal unchanged
        const Ray ray(Point3(0.0, 0.0, 0.0), Normalised(Vec3(1.0, 0.0, -1.0)));
        const Vec3 outward_normal(0.0, 0.0, 1.0);
        RayHitResult result;
        result.SetFaceNormal(ray, outward_normal);

        REQUIRE(result.m_is_front_facing == true);
        REQUIRE(result.m_normal.m_x == Approx(0.0));
        REQUIRE(result.m_normal.m_y == Approx(0.0));
        REQUIRE(result.m_normal.m_z == Approx(1.0));
    }

    SECTION("Ray leaving at 45 degrees from surface, back-facing")
    {
        // Ray direction: diagonal up-right, outward normal: +z
        // dot((1, 0, 1), (0, 0, 1)) = 1 > 0 -> back-facing, normal flipped
        const Ray ray(Point3(0.0, 0.0, 0.0), Normalised(Vec3(1.0, 0.0, 1.0)));
        const Vec3 outward_normal(0.0, 0.0, 1.0);
        RayHitResult result;
        result.SetFaceNormal(ray, outward_normal);

        REQUIRE(result.m_is_front_facing == false);
        REQUIRE(result.m_normal.m_x == Approx(0.0));
        REQUIRE(result.m_normal.m_y == Approx(0.0));
        REQUIRE(result.m_normal.m_z == Approx(-1.0));
    }
}

} // namespace ART
