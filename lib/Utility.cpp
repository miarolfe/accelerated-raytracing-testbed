// Copyright Mia Rolfe. All rights reserved.
#include <Utility.h>

#include <omp.h>
#if defined(_MSC_VER)
    #include <thread>
#endif // defined(_MSC_VER)

#include <Common.h>
#include <Constants.h>
#include <Logger.h>

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
    case AccelerationStructure::OCTREE:
        return "Octree";
    case AccelerationStructure::BSP_TREE:
        return "BSP tree";
    case AccelerationStructure::K_D_TREE:
        return "k-d tree";
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

void Init()
{
#if defined(_MSC_VER)
    std::size_t num_openmp_threads = std::thread::hardware_concurrency();
    if (num_openmp_threads <= 0)
    {
        num_openmp_threads = 1;
    }
    omp_set_num_threads(static_cast<int>(num_openmp_threads));

    Logger::Get().LogInfo("Using " + std::to_string(num_openmp_threads) + " threads for render");
#endif // defined(_MSC_VER)
}

} // namespace ART
