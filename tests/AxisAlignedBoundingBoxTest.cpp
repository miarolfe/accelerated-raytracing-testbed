// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <AxisAlignedBoundingBox.h>
#include <Constants.h>

namespace ART
{

TEST_CASE("AABB default constructor", "[AABB]")
{
    const AABB aabb;

    REQUIRE(aabb.m_x.m_min == Approx(infinity));
    REQUIRE(aabb.m_x.m_max == Approx(-infinity));
    REQUIRE(aabb.m_y.m_min == Approx(infinity));
    REQUIRE(aabb.m_y.m_max == Approx(-infinity));
    REQUIRE(aabb.m_z.m_min == Approx(infinity));
    REQUIRE(aabb.m_z.m_max == Approx(-infinity));
}

TEST_CASE("AABB proper constructor", "[AABB]")
{
    const AABB aabb(
        1.0, 2.0,
        3.0, 4.0,
        5.0, 6.0
    );

    REQUIRE(aabb.m_x.m_min == Approx(1.0));
    REQUIRE(aabb.m_x.m_max == Approx(2.0));
    REQUIRE(aabb.m_y.m_min == Approx(3.0));
    REQUIRE(aabb.m_y.m_max == Approx(4.0));
    REQUIRE(aabb.m_z.m_min == Approx(5.0));
    REQUIRE(aabb.m_z.m_max == Approx(6.0));
}

TEST_CASE("AABB copy constructor", "[AABB]")
{
    const AABB original(
        -1.0, 1.0,
        -2.0, 2.0,
        -3.0, 3.0
    );

    const AABB copy(original);

    REQUIRE(copy.m_x.m_min == Approx(-1.0));
    REQUIRE(copy.m_x.m_max == Approx(1.0));
    REQUIRE(copy.m_y.m_min == Approx(-2.0));
    REQUIRE(copy.m_y.m_max == Approx(2.0));
    REQUIRE(copy.m_z.m_min == Approx(-3.0));
    REQUIRE(copy.m_z.m_max == Approx(3.0));
}

TEST_CASE("AABB copy assignment operator", "[AABB]")
{
    const AABB source(
        10.0, 20.0,
        30.0, 40.0,
        50.0, 60.0
    );

    AABB target;
    target = source;

    REQUIRE(target.m_x.m_min == Approx(10.0));
    REQUIRE(target.m_x.m_max == Approx(20.0));
    REQUIRE(target.m_y.m_min == Approx(30.0));
    REQUIRE(target.m_y.m_max == Approx(40.0));
    REQUIRE(target.m_z.m_min == Approx(50.0));
    REQUIRE(target.m_z.m_max == Approx(60.0));
}

TEST_CASE("AABB copy assignment overwrites existing values", "[AABB]")
{
    AABB aabb1(
        1.0, 1.0,
        1.0, 1.0,
        1.0, 1.0
    );

    const AABB aabb2(
        -1.0, 2.0,
        -3.0, 4.0,
        -5.0, 6.0
    );

    aabb1 = aabb2;

    REQUIRE(aabb1.m_x.m_min == Approx(-1.0));
    REQUIRE(aabb1.m_x.m_max == Approx(2.0));
    REQUIRE(aabb1.m_y.m_min == Approx(-3.0));
    REQUIRE(aabb1.m_y.m_max == Approx(4.0));
    REQUIRE(aabb1.m_z.m_min == Approx(-5.0));
    REQUIRE(aabb1.m_z.m_max == Approx(6.0));
}

TEST_CASE("AABB component access via operator[]", "[AABB]")
{
    AABB non_const_aabb(
        1.0, 2.0,
        3.0, 4.0,
        5.0, 6.0
    );

    // Non-const access
    non_const_aabb[0].m_min = 10.0;
    non_const_aabb[1].m_max = 40.0;
    non_const_aabb[2].m_min = 50.0;

    REQUIRE(non_const_aabb.m_x.m_min == Approx(10.0));
    REQUIRE(non_const_aabb.m_x.m_max == Approx(2.0));
    REQUIRE(non_const_aabb.m_y.m_min == Approx(3.0));
    REQUIRE(non_const_aabb.m_y.m_max == Approx(40.0));
    REQUIRE(non_const_aabb.m_z.m_min == Approx(50.0));
    REQUIRE(non_const_aabb.m_z.m_max == Approx(6.0));

    // Const access
    const AABB const_aabb(
        -1.0, -2.0,
        -3.0, -4.0,
        -5.0, -6.0
    );

    REQUIRE(const_aabb[0].m_min == Approx(-1.0));
    REQUIRE(const_aabb[0].m_max == Approx(-2.0));
    REQUIRE(const_aabb[1].m_min == Approx(-3.0));
    REQUIRE(const_aabb[1].m_max == Approx(-4.0));
    REQUIRE(const_aabb[2].m_min == Approx(-5.0));
    REQUIRE(const_aabb[2].m_max == Approx(-6.0));
}

TEST_CASE("AABB LongestAxis returns correct axis index", "[AABB]")
{
    // X should be longest
    AABB aabb_x(
        0.0, 5.0,
        0.0, 3.0,
        0.0, 2.0
    );
    REQUIRE(aabb_x.LongestAxis() == 0);

    // Y should be longest
    AABB aabb_y(
        0.0, 2.0,
        0.0, 6.0,
        0.0, 4.0
    );
    REQUIRE(aabb_y.LongestAxis() == 1);

    // Z should be longest
    AABB aabb_z(
        0.0, 1.0,
        0.0, 2.0,
        0.0, 7.0
    );
    REQUIRE(aabb_z.LongestAxis() == 2);
}

} // namespace ART
