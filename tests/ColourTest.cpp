// Copyright Mia Rolfe. All rights reserved.
#include "../external/Catch2/catch.hpp"
#include "../lib/Colour.h"

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

} // namespace ART
