// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Core/Timer.h>

#include <thread>
#include <chrono>

namespace ART
{

TEST_CASE("Timer ElapsedMilliseconds is non-negative after immediate stop", "[Timer]")
{
    // Surpassing this would be concerning lol
    static constexpr double MAX_TIME_ELAPSED_MS = 50.0;

    Timer t;
    t.Start();
    t.Stop();

    REQUIRE(t.ElapsedMilliseconds() >= 0.0);
    REQUIRE(t.ElapsedMilliseconds() < MAX_TIME_ELAPSED_MS);
}

TEST_CASE("Timer ElapsedMilliseconds reflects actual elapsed time", "[Timer]")
{
    static constexpr double MAX_TIME_ELAPSED_MS = 100.0;

    Timer t;
    t.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    t.Stop();

    REQUIRE(t.ElapsedMilliseconds() >= 10.0);
    REQUIRE(t.ElapsedMilliseconds() < MAX_TIME_ELAPSED_MS);
}

TEST_CASE("Timer can be reused across multiple start/stop cycles", "[Timer]")
{
    Timer t;

    t.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    t.Stop();
    const double first = t.ElapsedMilliseconds();

    t.Start();
    t.Stop();
    const double second = t.ElapsedMilliseconds();

    // First measurement has delay, second doesn't
    REQUIRE(first > second);
}

} // namespace ART
