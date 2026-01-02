// Copyright Mia Rolfe. All rights reserved.
#include "../external/Catch2/catch.hpp"

#include "../lib/Ray.h"

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

    REQUIRE(ray.m_time == Approx(0.0));
}

TEST_CASE("Ray constructor without time", "[Ray]")
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

    REQUIRE(ray.m_time == Approx(0.0));
}

TEST_CASE("Ray constructor with time", "[Ray]")
{
    const Point3 origin(-1.0, -2.0, -3.0);
    const Vec3 direction(1.0, 0.0, 0.0);
    const double time = 2.5;

    const Ray ray(origin, direction, time);

    REQUIRE(ray.m_origin.m_x == Approx(-1.0));
    REQUIRE(ray.m_origin.m_y == Approx(-2.0));
    REQUIRE(ray.m_origin.m_z == Approx(-3.0));

    REQUIRE(ray.m_direction.m_x == Approx(1.0));
    REQUIRE(ray.m_direction.m_y == Approx(0.0));
    REQUIRE(ray.m_direction.m_z == Approx(0.0));

    REQUIRE(ray.m_time == Approx(2.5));
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
