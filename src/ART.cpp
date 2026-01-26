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

#include <Acceleration/AccelerationStructures.h>
#include <Core/Core.h>
#include <Geometry/Geometry.h>
#include <Materials/Materials.h>
#include <Maths/Maths.h>
#include <RayTracing/RayTracing.h>

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
