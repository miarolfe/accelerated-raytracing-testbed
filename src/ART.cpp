// Copyright Mia Rolfe. All rights reserved.

// Fallback to headless if neither defined
#if !defined (ART_GUI) && !defined (ART_HEADLESS)
    #define ART_HEADLESS
#endif // !defined (ART_GUI) && !defined (ART_HEADLESS)

#include <ArenaAllocator.h>
#if defined (ART_GUI)
    #include <GUIRunner.h>
#elif defined (ART_HEADLESS)
    #include <HeadlessRunner.h>
#endif // defined (ART_GUI)
#include <AxisAlignedBox.h>
#include <BoundingVolumeHierarchy.h>
#include <BSPTree.h>
#include <Camera.h>
#include <Colour.h>
#include <HierarchicalUniformGrid.h>
#include <KDTree.h>
#include <Logger.h>
#include <Material.h>
#include <Octree.h>
#include <RayHittableList.h>
#include <RenderCommon.h>
#include <Sphere.h>
#include <Texture.h>
#include <Timer.h>
#include <UniformGrid.h>
#include <Utility.h>
#include <Vec3.h>

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
