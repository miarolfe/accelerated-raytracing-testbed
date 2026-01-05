// Copyright Mia Rolfe. All rights reserved.
#include "../external/Catch2/catch.hpp"

#include "../lib/Constants.h"
#include "../lib/Utility.h"

namespace ART
{

TEST_CASE("DegreesToRadians converts degrees to radians correctly", "[Utility]")
{
    SECTION("Zero degrees")
    {
        REQUIRE(DegreesToRadians(0.0) == Approx(0.0));
    }

    SECTION("180 degrees = pi radians")
    {
        REQUIRE(DegreesToRadians(180.0) == Approx(pi));
    }

    SECTION("90 degrees = pi/2 radians")
    {
        REQUIRE(DegreesToRadians(90.0) == Approx(pi / 2.0));
    }

    SECTION("360 degrees = 2*pi radians")
    {
        REQUIRE(DegreesToRadians(360.0) == Approx(2.0 * pi));
    }

    SECTION("Negative degrees")
    {
        REQUIRE(DegreesToRadians(-90.0) == Approx(-pi / 2.0));
    }
}

} // namespace ART
