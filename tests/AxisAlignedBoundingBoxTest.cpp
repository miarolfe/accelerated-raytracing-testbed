// Copyright Mia Rolfe. All rights reserved.
#include "../external/Catch2/catch.hpp"

#include "../lib/AxisAlignedBoundingBox.h"

namespace ART
{

TEST_CASE("AABB default constructor", "[AABB]")
{
    const AABB box;

    REQUIRE(box.m_min_x == Approx(0.0));
    REQUIRE(box.m_max_x == Approx(0.0));
    REQUIRE(box.m_min_y == Approx(0.0));
    REQUIRE(box.m_max_y == Approx(0.0));
    REQUIRE(box.m_min_z == Approx(0.0));
    REQUIRE(box.m_max_z == Approx(0.0));
}

TEST_CASE("AABB proper constructor", "[AABB]")
{
    const AABB box(
        1.0, 2.0,
        3.0, 4.0,
        5.0, 6.0
    );

    REQUIRE(box.m_min_x == Approx(1.0));
    REQUIRE(box.m_max_x == Approx(2.0));
    REQUIRE(box.m_min_y == Approx(3.0));
    REQUIRE(box.m_max_y == Approx(4.0));
    REQUIRE(box.m_min_z == Approx(5.0));
    REQUIRE(box.m_max_z == Approx(6.0));
}

TEST_CASE("AABB copy constructor", "[AABB]")
{
    const AABB original(
        -1.0, 1.0,
        -2.0, 2.0,
        -3.0, 3.0
    );

    const AABB copy(original);

    REQUIRE(copy.m_min_x == Approx(-1.0));
    REQUIRE(copy.m_max_x == Approx(1.0));
    REQUIRE(copy.m_min_y == Approx(-2.0));
    REQUIRE(copy.m_max_y == Approx(2.0));
    REQUIRE(copy.m_min_z == Approx(-3.0));
    REQUIRE(copy.m_max_z == Approx(3.0));
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

    REQUIRE(target.m_min_x == Approx(10.0));
    REQUIRE(target.m_max_x == Approx(20.0));
    REQUIRE(target.m_min_y == Approx(30.0));
    REQUIRE(target.m_max_y == Approx(40.0));
    REQUIRE(target.m_min_z == Approx(50.0));
    REQUIRE(target.m_max_z == Approx(60.0));
}

TEST_CASE("AABB copy assignment overwrites existing values", "[AABB]")
{
    AABB box1(
        1.0, 1.0,
        1.0, 1.0,
        1.0, 1.0
    );

    const AABB box2(
        -1.0, 2.0,
        -3.0, 4.0,
        -5.0, 6.0
    );

    box1 = box2;

    REQUIRE(box1.m_min_x == Approx(-1.0));
    REQUIRE(box1.m_max_x == Approx(2.0));
    REQUIRE(box1.m_min_y == Approx(-3.0));
    REQUIRE(box1.m_max_y == Approx(4.0));
    REQUIRE(box1.m_min_z == Approx(-5.0));
    REQUIRE(box1.m_max_z == Approx(6.0));
}

} // namespace ART
