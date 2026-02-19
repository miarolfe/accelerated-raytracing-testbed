// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Core/TraversalStats.h>

namespace ART
{

TEST_CASE("TraversalCounters Reset zeroes all fields", "[TraversalStats]")
{
    TraversalCounters traversal_counters;
    traversal_counters.nodes_traversed = 100;
    traversal_counters.intersection_tests = 200;
    traversal_counters.rays_cast = 50;

    traversal_counters.Reset();

    REQUIRE(traversal_counters.nodes_traversed == 0);
    REQUIRE(traversal_counters.intersection_tests == 0);
    REQUIRE(traversal_counters.rays_cast == 0);
}

TEST_CASE("TraversalCounters operator+= accumulates correctly", "[TraversalStats]")
{
    TraversalCounters a;
    a.nodes_traversed = 10;
    a.intersection_tests = 20;
    a.rays_cast = 5;

    TraversalCounters b;
    b.nodes_traversed = 3;
    b.intersection_tests = 7;
    b.rays_cast = 2;

    a += b;

    REQUIRE(a.nodes_traversed == 13);
    REQUIRE(a.intersection_tests == 27);
    REQUIRE(a.rays_cast == 7);
}

TEST_CASE("TraversalStats averages compute correctly", "[TraversalStats]")
{
    TraversalStats stats;
    stats.total_nodes_traversed = 100;
    stats.total_intersection_tests = 50;
    stats.total_rays_cast = 10;

    REQUIRE(stats.AvgNodesTraversedPerRay() == Approx(10.0));
    REQUIRE(stats.AvgIntersectionTestsPerRay() == Approx(5.0));
}

TEST_CASE("TraversalStats averages return zero when no rays cast", "[TraversalStats]")
{
    TraversalStats stats;
    stats.total_nodes_traversed = 99;
    stats.total_intersection_tests = 42;
    stats.total_rays_cast = 0;

    REQUIRE(stats.AvgNodesTraversedPerRay() == Approx(0.0));
    REQUIRE(stats.AvgIntersectionTestsPerRay() == Approx(0.0));
}

TEST_CASE("Record helpers increment the thread-local traversal counters", "[TraversalStats]")
{
    tl_traversal_counters.Reset();

    RecordNodeTraversal();
    RecordNodeTraversal();
    RecordIntersectionTest();
    RecordRayCast();

    REQUIRE(tl_traversal_counters.nodes_traversed == 2);
    REQUIRE(tl_traversal_counters.intersection_tests == 1);
    REQUIRE(tl_traversal_counters.rays_cast == 1);

    // Reset for other tests, not constrained to this scope
    tl_traversal_counters.Reset();
}

} // namespace ART
