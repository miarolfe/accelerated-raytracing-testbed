// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Maths/Colour.h>

namespace ART
{

TEST_CASE("LinearToGamma converts linear colour values correctly", "[Colour]")
{
    SECTION("Zero remains zero")
    {
        REQUIRE(LinearToGamma(0.0) == Approx(0.0));
    }

    SECTION("One remains one")
    {
        REQUIRE(LinearToGamma(1.0) == Approx(1.0));
    }

    SECTION("Mid-range value #1 is gamma-corrected")
    {
        // RTIOW-style gamma correction is typically sqrt(x)
        double linear = 0.25;
        double expected = std::sqrt(linear);

        REQUIRE(LinearToGamma(linear) == Approx(expected));
    }

    SECTION("Mid-range value #1 is gamma-corrected")
    {
        double linear = 0.81;
        double expected = std::sqrt(linear);

        REQUIRE(LinearToGamma(linear) == Approx(expected));
    }
}

TEST_CASE("Colour arithmetic operations", "[Colour]")
{
    SECTION("Colour addition")
    {
        const Colour a(1.0, 0.5, 0.25);
        const Colour b(0.5, 0.5, 0.75);
        const Colour result = a + b;

        REQUIRE(result.m_x == Approx(1.5));
        REQUIRE(result.m_y == Approx(1.0));
        REQUIRE(result.m_z == Approx(1.0));
    }

    SECTION("Colour scalar multiply (pixel sample scale)")
    {
        const Colour c(4.0, 2.0, 1.0);
        const Colour result = c * 0.25;

        REQUIRE(result.m_x == Approx(1.0));
        REQUIRE(result.m_y == Approx(0.5));
        REQUIRE(result.m_z == Approx(0.25));
    }

    SECTION("Colour component-wise multiply (attenuation)")
    {
        const Colour attenuation(0.8, 0.6, 0.4);
        const Colour incoming(1.0, 0.5, 2.0);
        const Colour result = attenuation * incoming;

        REQUIRE(result.m_x == Approx(0.8));
        REQUIRE(result.m_y == Approx(0.3));
        REQUIRE(result.m_z == Approx(0.8));
    }

    SECTION("Colour += accumulation over samples")
    {
        Colour sum(0.0, 0.0, 0.0);
        sum += Colour(0.3, 0.3, 0.3);
        sum += Colour(0.3, 0.3, 0.3);
        sum += Colour(0.4, 0.4, 0.4);

        REQUIRE(sum.m_x == Approx(1.0));
        REQUIRE(sum.m_y == Approx(1.0));
        REQUIRE(sum.m_z == Approx(1.0));
    }
}

TEST_CASE("LinearToGamma clamps negative input to zero", "[Colour]")
{
    // Must not produce NaN or negative values
    const double result = LinearToGamma(-0.5);

    REQUIRE(!std::isnan(result));
    REQUIRE(result == Approx(0.0));
}

} // namespace ART
