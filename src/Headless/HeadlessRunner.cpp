// Copyright Mia Rolfe. All rights reserved.
#include <Headless/HeadlessRunner.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <RayTracing/Camera.h>
#include <Common/RenderCommon.h>
#include <Core/Utility.h>

namespace ART
{

void PrintHelpMsg(const char* program_name)
{
    std::cerr << "Usage: " << program_name << " [options]\n"
                << "Options:\n"
                << "  --width <pixels>       Screen width (default: 1280)\n"
                << "  --height <pixels>      Screen height (default: 720)\n"
                << "  --samples <count>      Samples per pixel (default: 100)\n"
                << "  --scene <scene_number> Scene to render (default: 1)\n"
                << "  --colour-seed <seed>   Seed for object colour RNG (default: 22052003, 0 = random)\n"
                << "  --position-seed <seed> Seed for object position RNG (default: 13012025, 0 = random)\n"
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
            if (out_params.scene < 1 || out_params.scene > 10)
            {
                std::cerr << "Error: --scene must be between 1 and 10\n";
                return false;
            }
        }
        else if (std::strcmp(argv[i], "--colour-seed") == 0)
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: --colour-seed requires a value\n";
                return false;
            }
            out_params.colour_seed = static_cast<uint32_t>(std::strtoul(argv[++i], nullptr, 10));
        }
        else if (std::strcmp(argv[i], "--position-seed") == 0)
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: --position-seed requires a value\n";
                return false;
            }
            out_params.position_seed = static_cast<uint32_t>(std::strtoul(argv[++i], nullptr, 10));
        }
        else
        {
            std::cerr << "Error: Unknown option '" << argv[i] << "'\n";
            PrintHelpMsg(argv[0]);
            return false;
        }
    }

    out_params.screen_width = (out_params.screen_width < MIN_RENDER_WIDTH) ? MIN_RENDER_WIDTH : out_params.screen_width;
    out_params.screen_width = (out_params.screen_width > MAX_RENDER_WIDTH) ? MAX_RENDER_WIDTH : out_params.screen_width;
    out_params.screen_height = (out_params.screen_height < MIN_RENDER_HEIGHT) ? MIN_RENDER_HEIGHT : out_params.screen_height;
    out_params.screen_height = (out_params.screen_height > MAX_RENDER_HEIGHT) ? MAX_RENDER_HEIGHT : out_params.screen_height;
    out_params.samples_per_pixel = (out_params.samples_per_pixel < MIN_SAMPLES_PER_PIXEL) ? MIN_SAMPLES_PER_PIXEL : out_params.samples_per_pixel;
    out_params.samples_per_pixel = (out_params.samples_per_pixel > MAX_SAMPLES_PER_PIXEL) ? MAX_SAMPLES_PER_PIXEL : out_params.samples_per_pixel;

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
    m_colour_seed = cli_params.colour_seed;
    m_position_seed = cli_params.position_seed;
}

HeadlessRunner::~HeadlessRunner()
{

}

void HeadlessRunner::Init()
{
    ART::Logger::Get().LogInfo("Initialising ART [Headless]");

    LogRenderConfig(m_camera_render_config, m_scene_number);

    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::NONE, m_colour_seed, m_position_seed);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::UNIFORM_GRID, m_colour_seed, m_position_seed);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::HIERARCHICAL_UNIFORM_GRID, m_colour_seed, m_position_seed);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::OCTREE, m_colour_seed, m_position_seed);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::BSP_TREE, m_colour_seed, m_position_seed);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::K_D_TREE, m_colour_seed, m_position_seed);
    RenderScene(m_camera_render_config, m_scene_number, AccelerationStructure::BOUNDING_VOLUME_HIERARCHY, m_colour_seed, m_position_seed);
}

void HeadlessRunner::Shutdown()
{
    ART::Logger::Get().LogInfo("Shutting down ART [Headless]");

    ART::Logger::Get().Flush();
}

} // namespace ART
