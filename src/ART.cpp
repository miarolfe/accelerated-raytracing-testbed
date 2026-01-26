// Copyright Mia Rolfe. All rights reserved.

// Fallback to headless if neither defined
#if !defined (ART_GUI) && !defined (ART_HEADLESS)
    #define ART_HEADLESS
#endif // !defined (ART_GUI) && !defined (ART_HEADLESS)

#include <Common/RenderCommon.h>
#if defined (ART_GUI)
    #include <GUI/GUIRunner.h>
#elif defined (ART_HEADLESS)
    #include <Headless/HeadlessRunner.h>
#endif // defined (ART_GUI)

#include <Acceleration/BoundingVolumeHierarchy.h>
#include <Acceleration/BSPTree.h>
#include <Acceleration/HierarchicalUniformGrid.h>
#include <Acceleration/KDTree.h>
#include <Acceleration/Octree.h>
#include <Acceleration/UniformGrid.h>
#include <Core/ArenaAllocator.h>
#include <Core/Logger.h>
#include <Core/Timer.h>
#include <Core/Utility.h>
#include <Geometry/AxisAlignedBox.h>
#include <Geometry/Sphere.h>
#include <Materials/Material.h>
#include <Materials/Texture.h>
#include <Maths/Colour.h>
#include <Maths/Vec3.h>
#include <RayTracing/Camera.h>
#include <RayTracing/RayHittableList.h>

int main(int argc, char* argv[])
{
#if defined (ART_GUI)
    ART::GUIRunner runner;
#elif defined (ART_HEADLESS)
    ART::HeadlessRunner runner(argc, argv);
#endif // ART_GUI
    ART::Init();

    runner.Init();

    runner.Shutdown();

    return 0;
}
