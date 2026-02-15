// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <cstdint>
#include <cstddef>

namespace ART
{

// Per-thread traversal efficiency counters
struct TraversalCounters
{
    uint64_t nodes_traversed = 0;
    uint64_t intersection_tests = 0;
    uint64_t rays_cast = 0;

    void Reset()
    {
        nodes_traversed = 0;
        intersection_tests = 0;
        rays_cast = 0;
    }

    TraversalCounters& operator+=(const TraversalCounters& other)
    {
        nodes_traversed += other.nodes_traversed;
        intersection_tests += other.intersection_tests;
        rays_cast += other.rays_cast;
        return *this;
    }
};

// Final traversal efficiency metrics
struct TraversalStats
{
    uint64_t total_nodes_traversed = 0;
    uint64_t total_intersection_tests = 0;
    uint64_t total_rays_cast = 0;

    double AvgNodesTraversedPerRay() const
    {
        return (total_rays_cast > 0) ? static_cast<double>(total_nodes_traversed) / total_rays_cast : 0.0;
    }

    double AvgIntersectionTestsPerRay() const
    {
        return (total_rays_cast > 0) ? static_cast<double>(total_intersection_tests) / total_rays_cast : 0.0;
    }
};

// Thread-local counters accessed during traversal
inline thread_local TraversalCounters tl_traversal_counters;

// Increment helpers for use in traversal code
inline void RecordNodeTraversal() { tl_traversal_counters.nodes_traversed++; }
inline void RecordIntersectionTest() { tl_traversal_counters.intersection_tests++; }
inline void RecordRayCast() { tl_traversal_counters.rays_cast++; }

} // namespace ART
