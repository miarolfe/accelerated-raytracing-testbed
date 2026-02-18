// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <cstdint>

#include <Common/RenderCommon.h>
#include <RayTracing/Camera.h>

namespace ART
{

struct CLIParams
{
public:
    std::size_t screen_width = 1280;
    std::size_t screen_height = 720;
    std::size_t samples_per_pixel = 100;
    int scene = 1;
    uint32_t colour_seed = DEFAULT_COLOUR_SEED;
    uint32_t position_seed = DEFAULT_POSITION_SEED;
};

void PrintHelpMsg(const char* program_name);

bool ParseCLIArgs(int argc, char* argv[], CLIParams& out_params);

CameraRenderConfig MakeCameraRenderConfig(const CLIParams& cli_params);

class HeadlessRunner
{
public:
    HeadlessRunner(int argc, char* argv[]);

    ~HeadlessRunner();

    void Init();

    void Shutdown();

protected:
    CameraRenderConfig m_camera_render_config;
    int m_scene_number = -1;
    uint32_t m_colour_seed = DEFAULT_COLOUR_SEED;
    uint32_t m_position_seed = DEFAULT_POSITION_SEED;
};

} // namespace ART
