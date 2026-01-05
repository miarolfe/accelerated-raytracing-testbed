// Copyright Mia Rolfe. All rights reserved.
#include "../external/Catch2/catch.hpp"

#include "../lib/Interval.h"

namespace ART
{

TEST_CASE("Interval constructors initialize correctly", "[Interval]")
{
    SECTION("Default constructor produces empty interval")
    {
        Interval interval;
        REQUIRE(interval.m_min == Approx(infinity));
        REQUIRE(interval.m_max == Approx(-infinity));
    }

    SECTION("Min/max constructor")
    {
        Interval interval(1.0, 5.0);
        REQUIRE(interval.m_min == Approx(1.0));
        REQUIRE(interval.m_max == Approx(5.0));
    }

    SECTION("Interval combination constructor")
    {
        Interval interval1(1.0, 3.0);
        Interval interval2(2.0, 5.0);
        Interval combined(interval1, interval2);

        REQUIRE(combined.m_min == Approx(1.0));
        REQUIRE(combined.m_max == Approx(5.0));
    }
}

TEST_CASE("Interval Size returns correct difference", "[Interval]")
{
    Interval interval(2.0, 5.0);
    REQUIRE(interval.Size() == Approx(3.0));

    Interval negative(5.0, 2.0);
    REQUIRE(negative.Size() == Approx(-3.0));
}

TEST_CASE("Interval Contains and Surrounds behave correctly", "[Interval]")
{
    Interval interval(1.0, 5.0);

    SECTION("Contains includes endpoints")
    {
        REQUIRE(interval.Contains(1.0) == true);
        REQUIRE(interval.Contains(5.0) == true);
        REQUIRE(interval.Contains(3.0) == true);
        REQUIRE(interval.Contains(0.0) == false);
        REQUIRE(interval.Contains(6.0) == false);
    }

    SECTION("Surrounds excludes endpoints")
    {
        REQUIRE(interval.Surrounds(1.0) == false);
        REQUIRE(interval.Surrounds(5.0) == false);
        REQUIRE(interval.Surrounds(3.0) == true);
    }
}

TEST_CASE("Interval Clamp works correctly", "[Interval]")
{
    Interval interval(1.0, 5.0);

    REQUIRE(interval.Clamp(0.0) == Approx(1.0));
    REQUIRE(interval.Clamp(6.0) == Approx(5.0));
    REQUIRE(interval.Clamp(3.0) == Approx(3.0));
}

TEST_CASE("Interval Expand correctly pads interval", "[Interval]")
{
    Interval interval(2.0, 4.0);
    Interval expanded = interval.Expand(2.0);

    REQUIRE(expanded.m_min == Approx(1.0));
    REQUIRE(expanded.m_max == Approx(5.0));
}

TEST_CASE("Interval static constants are correct", "[Interval]")
{
    REQUIRE(Interval::empty.m_min == Approx(infinity));
    REQUIRE(Interval::empty.m_max == Approx(-infinity));

    REQUIRE(Interval::universe.m_min == Approx(-infinity));
    REQUIRE(Interval::universe.m_max == Approx(infinity));
}

} // namespace ART
