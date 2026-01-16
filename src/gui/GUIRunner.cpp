// Copyright Mia Rolfe. All rights reserved.
#include <GUIRunner.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <string>

#include <Logger.h>
#include <RenderCommon.h>

namespace ART
{

void GUIRunner::Init()
{
    ART::Logger::Get().LogInfo("Initialising ART [GUI]");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        Logger::Get().LogFatal(std::string("Failed to initialize SDL: ") + SDL_GetError());
        return;
    }

    m_window = SDL_CreateWindow
    (
        "ART - Accelerated Raytracing Testbed",
        m_render_width,
        m_render_height,
        SDL_WINDOW_RESIZABLE
    );
    if (!m_window)
    {
        Logger::Get().LogFatal(std::string("Failed to create window: ") + SDL_GetError());
        return;
    }

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer)
    {
        Logger::Get().LogFatal(std::string("Failed to create renderer: ") + SDL_GetError());
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer);
    ImGui_ImplSDLRenderer3_Init(m_renderer);

    m_running = true;
    while (m_running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            const bool quit_requested = (event.type == SDL_EVENT_QUIT) || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE);
            if (quit_requested)
            {
                m_running = false;
            }
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        DrawUI();

        ImGui::Render();
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
        SDL_RenderPresent(m_renderer);

        if (m_render_requested)
        {
            CloseUI();
            RunRender();
            return;
        }
    }
}

void GUIRunner::DrawUI()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::Begin
    (
        "Main",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
    );

    if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const char* scenes[] = { "Scene 1", "Scene 2", "Scene 3", "Scene 4", "Scene 5", "Scene 6" };
        ImGui::Combo("Scene", &m_scene_number, scenes, 6);
        ImGui::InputInt("Width (px)", &m_render_width);
        ImGui::InputInt("Height (px)", &m_render_height);
        ImGui::InputInt("Samples per pixel", &m_samples_per_pixel);

        m_render_width = (m_render_width < 1) ? 1 : m_render_width;
        m_render_height = (m_render_height < 1) ? 1 : m_render_height;
        m_samples_per_pixel = (m_samples_per_pixel < 1) ? 1 : m_samples_per_pixel;
    }

    if (ImGui::CollapsingHeader("Acceleration Structures", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("None (Brute Force)", &m_use_acceleration_structure_none);
        ImGui::Checkbox("Uniform Grid", &m_use_acceleration_structure_uniform_grid);
        ImGui::Checkbox("Hierarchical Uniform Grid", &m_use_acceleration_structure_hierarchical_uniform_grid);
        ImGui::Checkbox("Octree", &m_use_acceleration_structure_octree);
        ImGui::Checkbox("BSP Tree", &m_use_acceleration_structure_bsp_tree);
        ImGui::Checkbox("K-D Tree", &m_use_acceleration_structure_k_d_tree);
        ImGui::Checkbox("Bounding Volume Hierarchy", &m_use_acceleration_structure_bounding_volume_hierarchy);
    }

    ImGui::Separator();
    if (ImGui::Button("Run Render", ImVec2(200, 40)))
    {
        m_render_requested = true;
    }

    ImGui::End();
}

void GUIRunner::CloseUI()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();

    m_renderer = nullptr;
    m_window = nullptr;
}

void GUIRunner::RunRender()
{
    CameraRenderConfig config
    {
        static_cast<std::size_t>(m_render_width),
        static_cast<std::size_t>(m_render_height),
        static_cast<std::size_t>(m_samples_per_pixel),
        25
    };
    int scene_number_one_indexed = m_scene_number + 1;

    LogRenderConfig(config, scene_number_one_indexed);

    if (m_use_acceleration_structure_none)
    {
        RenderScene(config, scene_number_one_indexed, AccelerationStructure::NONE);
    }
    if (m_use_acceleration_structure_uniform_grid)
    {
        RenderScene(config, scene_number_one_indexed, AccelerationStructure::UNIFORM_GRID);
    }
    if (m_use_acceleration_structure_hierarchical_uniform_grid)
    {
        RenderScene(config, scene_number_one_indexed, AccelerationStructure::HIERARCHICAL_UNIFORM_GRID);
    if (m_use_acceleration_structure_octree)
    {
        RenderScene(config, scene_number_one_indexed, AccelerationStructure::OCTREE);
    }
    if (m_use_acceleration_structure_bsp_tree)
    {
        RenderScene(config, scene_number_one_indexed, AccelerationStructure::BSP_TREE);
    }
    }
    if (m_use_acceleration_structure_k_d_tree)
    {
        RenderScene(config, scene_number_one_indexed, AccelerationStructure::K_D_TREE);
    }
    if (m_use_acceleration_structure_bounding_volume_hierarchy)
    {
        RenderScene(config, scene_number_one_indexed, AccelerationStructure::BOUNDING_VOLUME_HIERARCHY);
    }
}

void GUIRunner::Shutdown()
{
    ART::Logger::Get().LogInfo("Shutting down ART [GUI]");

    if (m_window)
    {
        CloseUI();
    }

    Logger::Get().Flush();
}

} // namespace ART
