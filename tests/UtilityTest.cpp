// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Core/Constants.h>
#include <Core/Utility.h>

#include <string>

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

TEST_CASE("AccelerationStructureToString returns non-empty string for each value", "[Utility]")
{
    REQUIRE(AccelerationStructureToString(AccelerationStructure::NONE) != "");
    REQUIRE(AccelerationStructureToString(AccelerationStructure::UNIFORM_GRID) != "");
    REQUIRE(AccelerationStructureToString(AccelerationStructure::HIERARCHICAL_UNIFORM_GRID) != "");
    REQUIRE(AccelerationStructureToString(AccelerationStructure::OCTREE) != "");
    REQUIRE(AccelerationStructureToString(AccelerationStructure::BSP_TREE) != "");
    REQUIRE(AccelerationStructureToString(AccelerationStructure::K_D_TREE) != "");
    REQUIRE(AccelerationStructureToString(AccelerationStructure::BOUNDING_VOLUME_HIERARCHY) != "");
}

TEST_CASE("AccelerationStructureToString returns distinct strings", "[Utility]")
{
    const std::string none_str  = AccelerationStructureToString(AccelerationStructure::NONE);
    const std::string uniform_grid_str  = AccelerationStructureToString(AccelerationStructure::UNIFORM_GRID);
    const std::string hierarchical_uniform_grid_str = AccelerationStructureToString(AccelerationStructure::HIERARCHICAL_UNIFORM_GRID);
    const std::string octree_str   = AccelerationStructureToString(AccelerationStructure::OCTREE);
    const std::string bsp_tree_str   = AccelerationStructureToString(AccelerationStructure::BSP_TREE);
    const std::string k_d_tree_str    = AccelerationStructureToString(AccelerationStructure::K_D_TREE);
    const std::string bounding_volume_hierarchy_str   = AccelerationStructureToString(AccelerationStructure::BOUNDING_VOLUME_HIERARCHY);

    REQUIRE(none_str != uniform_grid_str);
    REQUIRE(none_str != hierarchical_uniform_grid_str);
    REQUIRE(none_str != octree_str);
    REQUIRE(none_str != bsp_tree_str);
    REQUIRE(none_str != k_d_tree_str);
    REQUIRE(none_str != bounding_volume_hierarchy_str);
    REQUIRE(uniform_grid_str != hierarchical_uniform_grid_str);
    REQUIRE(bsp_tree_str != k_d_tree_str);
    REQUIRE(k_d_tree_str != bounding_volume_hierarchy_str);
}

TEST_CASE("RenderStats TotalTimeMilliseconds sums construction and render times", "[Utility]")
{
    SECTION("Both times zero")
    {
        RenderStats stats;
        stats.m_construction_time_ms = 0.0;
        stats.m_render_time_ms = 0.0;

        REQUIRE(stats.TotalTimeMilliseconds() == Approx(0.0));
    }

    SECTION("Typical values")
    {
        RenderStats stats;
        stats.m_construction_time_ms = 10.5;
        stats.m_render_time_ms = 99.5;

        REQUIRE(stats.TotalTimeMilliseconds() == Approx(110.0));
    }

    SECTION("Construction-only cost")
    {
        RenderStats stats;
        stats.m_construction_time_ms = 42.0;
        stats.m_render_time_ms = 0.0;

        REQUIRE(stats.TotalTimeMilliseconds() == Approx(42.0));
    }

    SECTION("Render-only cost")
    {
        RenderStats stats;
        stats.m_construction_time_ms = 0.0;
        stats.m_render_time_ms = 100.55;

        REQUIRE(stats.TotalTimeMilliseconds() == Approx(100.55));
    }
}

} // namespace ART
