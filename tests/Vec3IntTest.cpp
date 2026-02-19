// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Maths/Vec3Int.h>

namespace ART
{

TEST_CASE("Vec3Int default constructor initialises to zero", "[Vec3Int]")
{
    const Vec3Int v;

    REQUIRE(v.m_x == 0);
    REQUIRE(v.m_y == 0);
    REQUIRE(v.m_z == 0);
}

TEST_CASE("Vec3Int scalar constructor sets all components to the same value", "[Vec3Int]")
{
    const Vec3Int v(7);

    REQUIRE(v.m_x == 7);
    REQUIRE(v.m_y == 7);
    REQUIRE(v.m_z == 7);
}

TEST_CASE("Vec3Int component constructor sets x, y, z correctly", "[Vec3Int]")
{
    const Vec3Int v(1, 2, 3);

    REQUIRE(v.m_x == 1);
    REQUIRE(v.m_y == 2);
    REQUIRE(v.m_z == 3);
}

TEST_CASE("Vec3Int unary negation flips all components", "[Vec3Int]")
{
    const Vec3Int v(1, -2, 3);
    const Vec3Int neg = -v;

    REQUIRE(neg.m_x == -1);
    REQUIRE(neg.m_y == 2);
    REQUIRE(neg.m_z == -3);
}

TEST_CASE("Vec3Int operator[] accesses components by index", "[Vec3Int]")
{
    const Vec3Int v(4, 5, 6);

    REQUIRE(v[0] == 4);
    REQUIRE(v[1] == 5);
    REQUIRE(v[2] == 6);
}

TEST_CASE("Vec3Int operator[] by reference allows mutation", "[Vec3Int]")
{
    Vec3Int v(1, 2, 3);
    v[0] = 10;
    v[1] = 20;
    v[2] = 30;

    REQUIRE(v.m_x == 10);
    REQUIRE(v.m_y == 20);
    REQUIRE(v.m_z == 30);
}

TEST_CASE("Vec3Int operator+= accumulates component-wise", "[Vec3Int]")
{
    Vec3Int a(1, 2, 3);
    const Vec3Int b(4, 5, 6);
    a += b;

    REQUIRE(a.m_x == 5);
    REQUIRE(a.m_y == 7);
    REQUIRE(a.m_z == 9);
}

TEST_CASE("Vec3Int operator*= scales all components", "[Vec3Int]")
{
    Vec3Int v(2, 3, 4);
    v *= 3;

    REQUIRE(v.m_x == 6);
    REQUIRE(v.m_y == 9);
    REQUIRE(v.m_z == 12);
}

TEST_CASE("Vec3Int operator/= divides all components by scalar", "[Vec3Int]")
{
    SECTION("Even division")
    {
        Vec3Int v(4, 6, 8);
        v /= 2;

        REQUIRE(v.m_x == 2);
        REQUIRE(v.m_y == 3);
        REQUIRE(v.m_z == 4);
    }

    SECTION("Integer truncation towards zero")
    {
        Vec3Int v(5, 7, 9);
        v /= 2;

        REQUIRE(v.m_x == 2);
        REQUIRE(v.m_y == 3);
        REQUIRE(v.m_z == 4);
    }
}

TEST_CASE("Vec3Int free operator+ adds component-wise", "[Vec3Int]")
{
    const Vec3Int a(1, 2, 3);
    const Vec3Int b(4, 5, 6);
    const Vec3Int result = a + b;

    REQUIRE(result.m_x == 5);
    REQUIRE(result.m_y == 7);
    REQUIRE(result.m_z == 9);
}

TEST_CASE("Vec3Int free operator- subtracts component-wise", "[Vec3Int]")
{
    const Vec3Int a(5, 7, 9);
    const Vec3Int b(1, 2, 3);
    const Vec3Int result = a - b;

    REQUIRE(result.m_x == 4);
    REQUIRE(result.m_y == 5);
    REQUIRE(result.m_z == 6);
}

TEST_CASE("Vec3Int scalar multiply produces correct result", "[Vec3Int]")
{
    const Vec3Int v(1, 2, 3);
    const Vec3Int result = v * 4;

    REQUIRE(result.m_x == 4);
    REQUIRE(result.m_y == 8);
    REQUIRE(result.m_z == 12);
}

TEST_CASE("Vec3Int LengthSquared and Length are correct", "[Vec3Int]")
{
    const Vec3Int v(3, 4, 0);

    REQUIRE(v.LengthSquared() == Approx(25.0));
    REQUIRE(v.Length() == Approx(5.0));
}

} // namespace ART
