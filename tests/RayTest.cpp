// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Maths/Ray.h>

namespace ART
{

TEST_CASE("Ray default constructor", "[Ray]")
{
    const Ray ray;

    REQUIRE(ray.m_origin.m_x == Approx(0.0));
    REQUIRE(ray.m_origin.m_y == Approx(0.0));
    REQUIRE(ray.m_origin.m_z == Approx(0.0));

    REQUIRE(ray.m_direction.m_x == Approx(0.0));
    REQUIRE(ray.m_direction.m_y == Approx(0.0));
    REQUIRE(ray.m_direction.m_z == Approx(0.0));
}

TEST_CASE("Ray constructor", "[Ray]")
{
    const Point3 origin(1.0, 2.0, 3.0);
    const Vec3 direction(4.0, 5.0, 6.0);

    const Ray ray(origin, direction);

    REQUIRE(ray.m_origin.m_x == Approx(1.0));
    REQUIRE(ray.m_origin.m_y == Approx(2.0));
    REQUIRE(ray.m_origin.m_z == Approx(3.0));

    REQUIRE(ray.m_direction.m_x == Approx(4.0));
    REQUIRE(ray.m_direction.m_y == Approx(5.0));
    REQUIRE(ray.m_direction.m_z == Approx(6.0));
}

TEST_CASE("Ray inverse direction is precomputed correctly", "[Ray]")
{
    SECTION("Positive direction components")
    {
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(2.0, 4.0, 0.5));

        REQUIRE(ray.m_inverse_direction.m_x == Approx(0.5));
        REQUIRE(ray.m_inverse_direction.m_y == Approx(0.25));
        REQUIRE(ray.m_inverse_direction.m_z == Approx(2.0));
    }

    SECTION("Negative direction components")
    {
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(-1.0, -2.0, -0.5));

        REQUIRE(ray.m_inverse_direction.m_x == Approx(-1.0));
        REQUIRE(ray.m_inverse_direction.m_y == Approx(-0.5));
        REQUIRE(ray.m_inverse_direction.m_z == Approx(-2.0));
    }

    SECTION("Zero direction component produces infinity")
    {
        const Ray ray(Point3(0.0, 0.0, 0.0), Vec3(1.0, 0.0, 1.0));

        REQUIRE(ray.m_inverse_direction.m_x == Approx(1.0));
        REQUIRE(std::isinf(ray.m_inverse_direction.m_y));
        REQUIRE(ray.m_inverse_direction.m_z == Approx(1.0));
    }
}

TEST_CASE("Ray At(t) returns correct point along ray", "[Ray]")
{
    const Point3 origin(1.0, 2.0, 3.0);
    const Vec3 direction(2.0, 0.0, -1.0);
    const Ray ray(origin, direction);

    const Point3 p0 = ray.At(-1.0);
    REQUIRE(p0.m_x == Approx(-1.0));
    REQUIRE(p0.m_y == Approx(2.0));
    REQUIRE(p0.m_z == Approx(4.0));

    const Point3 p1 = ray.At(0.0);
    REQUIRE(p1.m_x == Approx(1.0));
    REQUIRE(p1.m_y == Approx(2.0));
    REQUIRE(p1.m_z == Approx(3.0));

    const Point3 p2 = ray.At(1.0);
    REQUIRE(p2.m_x == Approx(3.0));
    REQUIRE(p2.m_y == Approx(2.0));
    REQUIRE(p2.m_z == Approx(2.0));

    const Point3 p3 = ray.At(2.5);
    REQUIRE(p3.m_x == Approx(6.0));
    REQUIRE(p3.m_y == Approx(2.0));
    REQUIRE(p3.m_z == Approx(0.5));
}

} // namespace ART
