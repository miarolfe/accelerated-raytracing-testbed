// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Materials/Texture.h>
#include <Maths/Vec3.h>

namespace ART
{

TEST_CASE("SolidColourTexture returns the same colour for any UV and point", "[Texture]")
{
    SECTION("Colour constructor")
    {
        const SolidColourTexture t(Colour(0.4, 0.6, 0.8));

        const Colour a = t.Value(0.0, 0.0, Point3(0.0, 0.0, 0.0));
        const Colour b = t.Value(0.25, 0.75, Point3(5.0, -3.0, 2.0));
        const Colour c = t.Value(1.0, 1.0, Point3(-100.0, 100.0, 0.5));

        REQUIRE(a.m_x == Approx(0.4));
        REQUIRE(a.m_y == Approx(0.6));
        REQUIRE(a.m_z == Approx(0.8));

        REQUIRE(b.m_x == Approx(0.4));
        REQUIRE(b.m_y == Approx(0.6));
        REQUIRE(b.m_z == Approx(0.8));

        REQUIRE(c.m_x == Approx(0.4));
        REQUIRE(c.m_y == Approx(0.6));
        REQUIRE(c.m_z == Approx(0.8));
    }

    SECTION("Component constructor")
    {
        const SolidColourTexture t(0.1, 0.2, 0.3);

        const Colour a = t.Value(0.0, 0.0, Point3(0.0, 0.0, 0.0));
        const Colour b = t.Value(0.25, 0.75, Point3(5.0, -3.0, 2.0));
        const Colour c = t.Value(1.0, 1.0, Point3(-100.0, 100.0, 0.5));

        REQUIRE(a.m_x == Approx(0.1));
        REQUIRE(a.m_y == Approx(0.2));
        REQUIRE(a.m_z == Approx(0.3));

        REQUIRE(b.m_x == Approx(0.1));
        REQUIRE(b.m_y == Approx(0.2));
        REQUIRE(b.m_z == Approx(0.3));

        REQUIRE(c.m_x == Approx(0.1));
        REQUIRE(c.m_y == Approx(0.2));
        REQUIRE(c.m_z == Approx(0.3));
    }
}

TEST_CASE("CheckerTexture alternates between even and odd textures", "[Texture]")
{
    // scale=1.0 means m_inverse_scale=1.0, transitions at every integer boundary
    // For point (x, y, z): sum = floor(x) + floor(y) + floor(z)
    // even sum -> even texture (red), odd sum -> odd texture (blue)

    // Red
    SolidColourTexture even_texture(Colour(1.0, 0.0, 0.0));
    // Blue
    SolidColourTexture odd_texture(Colour(0.0, 0.0, 1.0));
    const CheckerTexture checker_texture(1.0, &even_texture, &odd_texture);

    SECTION("Point in even cell: floor(0.5) + floor(0.5) + floor(0.5) = 0 + 0 + 0 = 0 (even)")
    {
        const Colour result = checker_texture.Value(0.0, 0.0, Point3(0.5, 0.5, 0.5));
        REQUIRE(result.m_x == Approx(1.0)); // Red
        REQUIRE(result.m_z == Approx(0.0)); // Red
    }

    SECTION("Point in odd cell: floor(1.5) + floor(0.5) + floor(0.5) = 1 + 0 + 0 = 1 (odd)")
    {
        const Colour result = checker_texture.Value(0.0, 0.0, Point3(1.5, 0.5, 0.5));
        REQUIRE(result.m_z == Approx(1.0)); // Blue
        REQUIRE(result.m_x == Approx(0.0)); // Blue
    }

    SECTION("Point in even cell: floor(1.5) + floor(1.5) + floor(0.5) = 1 + 1 + 0 = 2 (even)")
    {
        const Colour result = checker_texture.Value(0.0, 0.0, Point3(1.5, 1.5, 0.5));
        REQUIRE(result.m_x == Approx(1.0)); // Red
    }

    SECTION("Point in odd cell: floor(1.5)+floor(1.5)+floor(1.5) = 1 + 1 + 1 = 3 (odd)")
    {
        const Colour result = checker_texture.Value(0.0, 0.0, Point3(1.5, 1.5, 1.5));
        REQUIRE(result.m_z == Approx(1.0)); // Blue
    }

    SECTION("UV coordinates are ignored, only world point matters")
    {
        // Same point, different UVs should give same result
        const Colour a = checker_texture.Value(0.0, 0.0, Point3(0.5, 0.5, 0.5));
        const Colour b = checker_texture.Value(0.99, 0.99, Point3(0.5, 0.5, 0.5));
        REQUIRE(a.m_x == Approx(b.m_x));
        REQUIRE(a.m_y == Approx(b.m_y));
        REQUIRE(a.m_z == Approx(b.m_z));
    }
}

TEST_CASE("CheckerTexture scale parameter controls transition frequency", "[Texture]")
{
    // scale=2.0 means m_inverse_scale=0.5, transitions every 2 units
    // floor(0.5 * x), so transitions at x = 0, 2, 4, and so on
    SolidColourTexture even_texture(Colour(1.0, 1.0, 1.0)); // White
    SolidColourTexture odd_texture(Colour(0.0, 0.0, 0.0));  // Black
    const CheckerTexture checker_texture(2.0, &even_texture, &odd_texture);

    SECTION("Point at x = 0.5: floor(0.5 * 0.5) = floor(0.25) = 0, sum = 0 (even) -> white")
    {
        const Colour result = checker_texture.Value(0.0, 0.0, Point3(0.5, 0.0, 0.0));
        REQUIRE(result.m_x == Approx(1.0));
    }

    SECTION("Point at x = 3.0: floor(0.5 * 3.0) = floor(1.5) = 1, sum = 1 (odd) -> black")
    {
        const Colour result = checker_texture.Value(0.0, 0.0, Point3(3.0, 0.0, 0.0));
        REQUIRE(result.m_x == Approx(0.0));
    }

    SECTION("Point at x = 5.0: floor(0.5 * 5.0) = floor(2.5) = 2, sum = 2 (even) -> white")
    {
        const Colour result = checker_texture.Value(0.0, 0.0, Point3(5.0, 0.0, 0.0));
        REQUIRE(result.m_x == Approx(1.0));
    }
}

TEST_CASE("CheckerTexture with negative coordinates", "[Texture]")
{
    // C++ % operator on negative values: (-1) % 2 == -1 (not 1)
    // Test documents the actual behaviour for negative coords
    SolidColourTexture even_texture(Colour(1.0, 0.0, 0.0)); // Red
    SolidColourTexture odd_texture(Colour(0.0, 0.0, 1.0));  // Blue
    const CheckerTexture checker_texture(1.0, &even_texture, &odd_texture);

    // floor(-0.5) = -1: sum = -1 + 0 + 0 = -1, (-1 % 2) == -1 in C++ not 0
    // So, is_even = (-1 == 0) = false -> odd texture (Blue)
    SECTION("Negative x coordinate: floor(-0.5) = -1, sum = -1, (-1 % 2) = -1, not even -> blue")
    {
        const Colour result = checker_texture.Value(0.0, 0.0, Point3(-0.5, 0.5, 0.5));
        REQUIRE(result.m_z == Approx(1.0)); // Blue (odd)
    }

    // floor(-1.5) = -2: sum = -2 + 0 + 0 = -2, (-2 % 2) == 0 in C++ -> even texture (Red)
    SECTION("Negative x coordinate: floor(-1.5)=-2, sum=-2, (-2%2)=0, even -> red")
    {
        const Colour result = checker_texture.Value(0.0, 0.0, Point3(-1.5, 0.5, 0.5));
        REQUIRE(result.m_x == Approx(1.0)); // Red (even)
    }
}

TEST_CASE("ImageTexture returns fallback colour when image fails to load", "[Texture]")
{
    const ImageTexture t("no_image_here.png");

    const Colour result = t.Value(0.5, 0.5, Point3(0.0, 0.0, 0.0));

    REQUIRE(result.m_x == Approx(0.0));
    REQUIRE(result.m_y == Approx(1.0));
    REQUIRE(result.m_z == Approx(1.0));
}

} // namespace ART
