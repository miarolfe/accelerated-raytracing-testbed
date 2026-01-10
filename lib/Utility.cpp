// Copyright Mia Rolfe. All rights reserved.
#include <Utility.h>

#include <Common.h>
#include <Constants.h>

namespace ART
{

double DegreesToRadians(double degrees)
{
    return degrees * pi / 180.0;
}

const std::string AccelerationStructureToString(AccelerationStructure acceleration_structure)
{
    switch (acceleration_structure)
    {
    case AccelerationStructure::NONE:
        return "None";
    case AccelerationStructure::UNIFORM_GRID:
        return "Uniform grid";
    case AccelerationStructure::HIERARCHICAL_UNIFORM_GRID:
        return "Hierarchical uniform grid";
    case AccelerationStructure::BOUNDING_VOLUME_HIERARCHY:
        return "Bounding volume hierarchy";
    }

    assert(false);
    return "";
}

double RenderStats::TotalTimeMilliseconds() const
{
    return m_construction_time_ms + m_render_time_ms;
}

} // namespace ART
