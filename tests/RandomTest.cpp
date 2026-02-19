// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Core/Random.h>

namespace ART
{

static constexpr int NUM_ITERATIONS = 100000;

TEST_CASE("RandomCanonicalDouble returns values in [0, 1)", "[Random]")
{
    double v_low = 1.1;
    double v_high = -0.1;

    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        const double v = RandomCanonicalDouble();
        v_low = std::min(v_low, v);
        v_high = std::max(v_high, v);
    }

    REQUIRE(v_low < 1.0);
    REQUIRE(v_high >= 0.0);
}

TEST_CASE("RandomDouble returns values in [min, max)", "[Random]")
{
    SECTION("Positive range")
    {
        const double min = 2.0;
        const double max = 5.0;

        double v_low = max + 0.1;
        double v_high = min - 0.1;

        for (int i = 0; i < NUM_ITERATIONS; i++)
        {
            const double v = RandomDouble(min, max);
            v_low = std::min(v_low, v);
            v_high = std::max(v_high, v);
        }

        REQUIRE(v_low < max);
        REQUIRE(v_high >= min);
    }

    SECTION("Range spanning zero")
    {
        const double min = -5.0;
        const double max = 10.0;

        double v_low = max + 0.1;
        double v_high = min - 0.1;

        for (int i = 0; i < NUM_ITERATIONS; i++)
        {
            const double v = RandomDouble(min, max);
            v_low = std::min(v_low, v);
            v_high = std::max(v_high, v);
        }

        REQUIRE(v_low < max);
        REQUIRE(v_high >= min);
    }
}

TEST_CASE("SeedColourRNG produces deterministic sequence", "[Random]")
{
    SeedColourRNG(12345);
    const double a = RandomColourDouble();
    const double b = RandomColourDouble();

    SeedColourRNG(12345);
    const double c = RandomColourDouble();
    const double d = RandomColourDouble();

    REQUIRE(a == Approx(c));
    REQUIRE(b == Approx(d));
}

TEST_CASE("SeedPositionRNG produces deterministic sequence", "[Random]")
{
    SeedPositionRNG(99999);
    const double a = RandomPositionDouble(0.0, 100.0);
    const double b = RandomPositionDouble(0.0, 100.0);

    SeedPositionRNG(99999);
    const double c = RandomPositionDouble(0.0, 100.0);
    const double d = RandomPositionDouble(0.0, 100.0);

    REQUIRE(a == Approx(c));
    REQUIRE(b == Approx(d));
}

TEST_CASE("Colour and position RNG streams are independent", "[Random]")
{
    SeedColourRNG(42);
    SeedPositionRNG(42);

    // Draw from colour stream first
    const double colour_first = RandomColourDouble();

    SeedColourRNG(42);
    SeedPositionRNG(42);
    RandomPositionDouble(0.0, 1.0);
    const double colour_after_position = RandomColourDouble();

    // Drawing from position stream must not affect the colour stream
    REQUIRE(colour_first == Approx(colour_after_position));
}

TEST_CASE("RandomColourDouble returns values in [0, 1)", "[Random]")
{
    SeedColourRNG(1);

    double v_low = 1.1;
    double v_high = -0.1;

    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        const double v = RandomColourDouble();
        v_low = std::min(v_low, v);
        v_high = std::max(v_high, v);
    }

    REQUIRE(v_low < 1.0);
    REQUIRE(v_high >= 0.0);
}

TEST_CASE("RandomPositionDouble returns values in [min, max)", "[Random]")
{
    SeedPositionRNG(1);

    const double min = -10.0;
    const double max = 10.0;

    double v_low = max + 0.1;
    double v_high = min - 0.1;

    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        const double v = RandomDouble(min, max);
        v_low = std::min(v_low, v);
        v_high = std::max(v_high, v);
    }

    REQUIRE(v_low < max);
    REQUIRE(v_high >= min);
}

} // namespace ART
