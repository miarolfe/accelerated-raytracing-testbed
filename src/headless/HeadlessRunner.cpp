// Copyright Mia Rolfe. All rights reserved.
#include <HeadlessRunner.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <Camera.h>
#include <RenderCommon.h>
#include <Utility.h>

namespace ART
{

void PrintHelpMsg(const char* program_name)
{
    std::cerr << "Usage: " << program_name << " [options]\n"
                << "Options:\n"
                << "  --width <pixels>       Screen width (default: 1280)\n"
                << "  --height <pixels>      Screen height (default: 720)\n"
                << "  --samples <count>      Samples per pixel (default: 100)\n"
                << "  --scene <scene_number> Scene to render (default: 5)\n"
                << "  --help                 Show this help message\n";
}

bool ParseCLIArgs(int argc, char* argv[], CLIParams& out_params)
{
    for (int i = 1; i < argc; i++)
    {
        if (std::strcmp(argv[i], "--help") == 0)
        {
            PrintHelpMsg(argv[0]);
            return false;
        }
        else if (std::strcmp(argv[i], "--width") == 0)
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: --width requires a value\n";
                return false;
            }
            out_params.screen_width = static_cast<std::size_t>(std::atoi(argv[++i]));
        }
        else if (std::strcmp(argv[i], "--height") == 0)
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: --height requires a value\n";
                return false;
            }
            out_params.screen_height = static_cast<std::size_t>(std::atoi(argv[++i]));
        }
        else if (std::strcmp(argv[i], "--samples") == 0)
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: --samples requires a value\n";
                return false;
            }
            out_params.samples_per_pixel = static_cast<std::size_t>(std::atoi(argv[++i]));
        }
        else if (std::strcmp(argv[i], "--scene") == 0)
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: --scene requires a value\n";
                return false;
            }
            out_params.scene = std::atoi(argv[++i]);
            if (out_params.scene < 1 || out_params.scene > 6)
            {
                std::cerr << "Error: --scene must be between 1 and 6\n";
                return false;
            }
        }
        else
        {
            std::cerr << "Error: Unknown option '" << argv[i] << "'\n";
            PrintHelpMsg(argv[0]);
            return false;
        }
    }
    return true;
}

CameraRenderConfig MakeCameraRenderConfig(const CLIParams& cli_params)
{
    return CameraRenderConfig
    {
        cli_params.screen_width,
        cli_params.screen_height,
        cli_params.samples_per_pixel,
        25
    };
}

HeadlessRunner::HeadlessRunner(int argc, char* argv[])
{
    CLIParams cli_params;
    ParseCLIArgs(argc, argv, cli_params);
    m_camera_render_config = MakeCameraRenderConfig(cli_params);
    m_scene_number = cli_params.scene;
}

HeadlessRunner::~HeadlessRunner()
{

}

void HeadlessRunner::Init()
{
    ART::Logger::Get().LogInfo("Booting up");

    LogRenderConfig(m_camera_render_config, m_scene_number);

    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::NONE);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::UNIFORM_GRID);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::HIERARCHICAL_UNIFORM_GRID);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::OCTREE);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::BSP_TREE);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::K_D_TREE);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::BOUNDING_VOLUME_HIERARCHY);
}

void HeadlessRunner::Shutdown()
{
    ART::Logger::Get().LogInfo("Shutting down");

    ART::Logger::Get().Flush();
}

} // namespace ART
