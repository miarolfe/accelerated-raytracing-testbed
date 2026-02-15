// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <string>

namespace ART
{

double DegreesToRadians(double degrees);

enum class AccelerationStructure
{
    NONE,
    UNIFORM_GRID,
    HIERARCHICAL_UNIFORM_GRID,
    OCTREE,
    BSP_TREE,
    K_D_TREE,
    BOUNDING_VOLUME_HIERARCHY
};

const std::string AccelerationStructureToString(AccelerationStructure acceleration_structure);

struct RenderStats
{
public:
    AccelerationStructure m_acceleration_structure;
    double m_construction_time_ms = 0.0;
    double m_render_time_ms = 0.0;
    std::size_t m_memory_used_bytes = 0;

    double TotalTimeMilliseconds() const;
};

void Init();

} // namespace ART
