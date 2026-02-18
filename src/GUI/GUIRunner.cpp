// Copyright Mia Rolfe. All rights reserved.
#include <GUI/GUIRunner.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <cstdio>
#include <string>

#include <Core/Logger.h>
#include <Common/RenderCommon.h>

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
                // Cancel in-progress renders before quitting
                if (m_render_state == RenderState::RENDERING)
                {
                    CancelRenders();
                }
                m_running = false;
            }
        }

        UpdateRenderState();

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        DrawUI();

        ImGui::Render();
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
        SDL_RenderPresent(m_renderer);

        // Delay GUI refresh to avoid slowing down render
        // Kind of a hack and should probably have a proper timer system
        static constexpr int window_refresh_delay_ms = 16;
        SDL_Delay(window_refresh_delay_ms);
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

    switch (m_render_state)
    {
        case RenderState::IDLE:
        case RenderState::COMPLETED:
            DrawSettingsUI();
            break;
        case RenderState::RENDERING:
            DrawRenderingUI();
            break;
    }

    ImGui::End();
}

void GUIRunner::DrawSettingsUI()
{
    if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const char* scenes[] = {
            "Scene 1 (Three sphere clusters)",
            "Scene 2 (Uniform dense field)",
            "Scene 3 (Sparse clusters in void)",
            "Scene 4 (Extreme size variation)",
            "Scene 5 (Long corridor)",
            "Scene 6 (Centroid co-location)",
            "Scene 7 (Flat plane distribution)",
            "Scene 8 (Diagonal wall)",
            "Scene 9 (High object count)",
            "Scene 10 (Overlapping box city)"
        };
        ImGui::Combo("Scene", &m_scene_number, scenes, 10);
        ImGui::InputInt("Width (px)", &m_render_width);
        ImGui::InputInt("Height (px)", &m_render_height);
        ImGui::InputInt("Samples per pixel", &m_samples_per_pixel);
        ImGui::InputInt("Colour seed (0 = random)", &m_colour_seed);
        ImGui::InputInt("Position seed (0 = random)", &m_position_seed);

        m_render_width = (m_render_width < MIN_RENDER_WIDTH) ? MIN_RENDER_WIDTH : m_render_width;
        m_render_width = (m_render_width > MAX_RENDER_WIDTH) ? MAX_RENDER_WIDTH : m_render_width;
        m_render_height = (m_render_height < MIN_RENDER_HEIGHT) ? MIN_RENDER_HEIGHT : m_render_height;
        m_render_height = (m_render_height > MAX_RENDER_HEIGHT) ? MAX_RENDER_HEIGHT : m_render_height;
        m_samples_per_pixel = (m_samples_per_pixel < MIN_SAMPLES_PER_PIXEL) ? MIN_SAMPLES_PER_PIXEL : m_samples_per_pixel;
        m_samples_per_pixel = (m_samples_per_pixel > MAX_SAMPLES_PER_PIXEL) ? MAX_SAMPLES_PER_PIXEL : m_samples_per_pixel;
        m_colour_seed = (m_colour_seed < 0) ? 0 : m_colour_seed;
        m_position_seed = (m_position_seed < 0) ? 0 : m_position_seed;
    }

    if (ImGui::CollapsingHeader("Acceleration Structures", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("None (brute force)", &m_use_acceleration_structure_none);
        ImGui::Checkbox("Uniform grid", &m_use_acceleration_structure_uniform_grid);
        ImGui::Checkbox("Hierarchical uniform grid", &m_use_acceleration_structure_hierarchical_uniform_grid);
        ImGui::Checkbox("Octree", &m_use_acceleration_structure_octree);
        ImGui::Checkbox("BSP tree", &m_use_acceleration_structure_bsp_tree);
        ImGui::Checkbox("k-d tree", &m_use_acceleration_structure_k_d_tree);
        ImGui::Checkbox("Bounding volume hierarchy", &m_use_acceleration_structure_bounding_volume_hierarchy);
    }

    ImGui::Separator();

    if (m_render_state == RenderState::COMPLETED)
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Render complete!");
        DrawResultsUI();
        ImGui::Separator();
    }

    if (ImGui::Button("Run render", ImVec2(200, 40)))
    {
        StartRenderQueue();
    }
}

void GUIRunner::DrawRenderingUI()
{
    if (m_current_job_index >= m_render_queue.size())
    {
        return;
    }

    RenderJob& current_job = m_render_queue[m_current_job_index];
    RenderContext& render_context = current_job.context;

    // Header with current acceleration structure
    const std::string header = "Rendering: " + AccelerationStructureToString(current_job.context.acceleration_structure);
    ImGui::Text("%s", header.c_str());

    const std::size_t num_completed_rows = render_context.num_completed_rows.load(std::memory_order_relaxed);
    const std::size_t total_rows = render_context.total_rows.load(std::memory_order_relaxed);
    const float progress = (total_rows > 0) ? static_cast<float>(num_completed_rows) / static_cast<float>(total_rows) : 0.0f;
    ImGui::ProgressBar(progress, ImVec2(-1, 0));

    ImGui::Text("Progress: %zu / %zu rows (%.1f%%)", num_completed_rows, total_rows, static_cast<double>(progress) * 100.0);
    ImGui::Text("Job %zu of %zu", m_current_job_index + 1, m_render_queue.size());

    ImGui::Separator();

    // Live preview
    UpdatePreviewTexture();
    if (m_preview_texture)
    {
        ImVec2 available_area = ImGui::GetContentRegionAvail();
        const float preview_image_scale = std::max
        (
            0.1f,
            std::min(available_area.x / m_preview_width, (available_area.y - 60) / m_preview_height)
        );

        ImVec2 preview_size(m_preview_width * preview_image_scale, m_preview_height * preview_image_scale);
        ImGui::Image(static_cast<ImTextureRef>(m_preview_texture), preview_size);
    }

    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0, 0.0, 0.0, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.98f, 0.26f, 0.26f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.98f, 0.06f, 0.06f, 1.00f));
    if (ImGui::Button("Cancel", ImVec2(200, 40)))
    {
        CancelRenders();
    }
    ImGui::PopStyleColor(3);
}

std::string GUIRunner::FormatMemoryUsed(std::size_t bytes)
{
    char buffer[64];
    if (bytes >= ONE_MEGABYTE)
    {
        std::snprintf(buffer, sizeof(buffer), "%.2f MB", static_cast<double>(bytes) / ONE_MEGABYTE);
    }
    else if (bytes >= ONE_KILOBYTE)
    {
        std::snprintf(buffer, sizeof(buffer), "%.2f KB", static_cast<double>(bytes) / ONE_KILOBYTE);
    }
    else
    {
        std::snprintf(buffer, sizeof(buffer), "%zu B", bytes);
    }
    return buffer;
}

void GUIRunner::DrawResultsUI()
{
    if (m_completed_stats.empty())
    {
        return;
    }

    if (ImGui::BeginTable("RenderResults", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Structure");
        ImGui::TableSetupColumn("Construction (ms)");
        ImGui::TableSetupColumn("Render (ms)");
        ImGui::TableSetupColumn("Total (ms)");
        ImGui::TableSetupColumn("Memory used");
        ImGui::TableSetupColumn("Avg nodes/ray");
        ImGui::TableSetupColumn("Avg tests/ray");
        ImGui::TableHeadersRow();

        for (const RenderStats& stats : m_completed_stats)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%s", AccelerationStructureToString(stats.m_acceleration_structure).c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", stats.m_construction_time_ms);

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", stats.m_render_time_ms);

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", stats.TotalTimeMilliseconds());

            ImGui::TableNextColumn();
            ImGui::Text("%s", FormatMemoryUsed(stats.m_memory_used_bytes).c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", stats.m_traversal_stats.AvgNodesTraversedPerRay());

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", stats.m_traversal_stats.AvgIntersectionTestsPerRay());
        }

        ImGui::EndTable();
    }

    if (ImGui::Button("Copy as Markdown"))
    {
        std::ostringstream md;
        md << std::fixed << std::setprecision(2);
        md << "| Structure | Construction (ms) | Render (ms) | Total (ms) | Memory used | Avg nodes/ray | Avg tests/ray |\n";
        md << "| --- | --- | --- | --- | --- | --- | --- |\n";
        for (const RenderStats& stats : m_completed_stats)
        {
            md << "| " << AccelerationStructureToString(stats.m_acceleration_structure)
               << " | " << stats.m_construction_time_ms
               << " | " << stats.m_render_time_ms
               << " | " << stats.TotalTimeMilliseconds()
               << " | " << FormatMemoryUsed(stats.m_memory_used_bytes)
               << " | " << stats.m_traversal_stats.AvgNodesTraversedPerRay()
               << " | " << stats.m_traversal_stats.AvgIntersectionTestsPerRay()
               << " |\n";
        }
        SDL_SetClipboardText(md.str().c_str());
    }
}

void GUIRunner::UpdateRenderState()
{
    if (m_render_state != RenderState::RENDERING)
    {
        return;
    }

    if (m_current_job_index >= m_render_queue.size())
    {
        return;
    }

    RenderContext& render_context = m_render_queue[m_current_job_index].context;

    if (!render_context.render_complete.load(std::memory_order_acquire))
    {
        return;
    }

    // Wait for render thread to finish
    if (m_render_thread.joinable())
    {
        m_render_thread.join();
    }

    // Collect stats from completed job
    {
        RenderContext& completed_ctx = render_context;
        if (!completed_ctx.was_cancelled.load(std::memory_order_relaxed))
        {
            RenderStats stats;
            stats.m_acceleration_structure = completed_ctx.acceleration_structure;
            stats.m_construction_time_ms = completed_ctx.construction_time_ms;
            stats.m_render_time_ms = completed_ctx.render_time_ms;
            stats.m_memory_used_bytes = completed_ctx.memory_used_bytes;
            stats.m_traversal_stats = completed_ctx.traversal_stats;
            m_completed_stats.push_back(stats);
        }
    }

    // Move to next job
    m_current_job_index++;
    if (m_current_job_index < m_render_queue.size())
    {
        StartNextRender();
    }
    else
    {
        // All jobs complete
        m_render_state = RenderState::COMPLETED;
        m_render_queue.clear();
    }
}

void GUIRunner::UpdatePreviewTexture()
{
    // No render to preview
    if (m_current_job_index >= m_render_queue.size())
    {
        return;
    }

    RenderContext& ctx = m_render_queue[m_current_job_index].context;

    const uint8_t* image_data = ctx.camera.GetImageBuffer();
    int width = static_cast<int>(ctx.camera.GetImageWidth());
    int height = static_cast<int>(ctx.camera.GetImageHeight());

    // No preview to draw
    if (!image_data || width <= 0 || height <= 0)
    {
        return;
    }

    const bool should_recreate_texture = (m_preview_texture == nullptr || m_preview_width != width || m_preview_height != height);

    if (should_recreate_texture)
    {
        if (m_preview_texture)
        {
            SDL_DestroyTexture(m_preview_texture);
        }

        m_preview_texture = SDL_CreateTexture
        (
            m_renderer,
            SDL_PIXELFORMAT_RGB24,
            SDL_TEXTUREACCESS_STREAMING,
            width,
            height
        );
        m_preview_width = width;
        m_preview_height = height;
    }

    if (m_preview_texture)
    {
        SDL_UpdateTexture(m_preview_texture, nullptr, image_data, width * 3);
    }
}

void GUIRunner::StartRenderQueue()
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

    // Cast from int (ImGui expects int for UI values)
    const uint32_t colour_seed = static_cast<uint32_t>(m_colour_seed);
    const uint32_t position_seed = static_cast<uint32_t>(m_position_seed);

    m_render_queue.clear();
    m_completed_stats.clear();
    m_current_job_index = 0;

    if (m_use_acceleration_structure_none)
    {
        RenderJob job;
        job.context = CreateAsyncRenderContext(config, scene_number_one_indexed, AccelerationStructure::NONE, colour_seed, position_seed);
        m_render_queue.push_back(std::move(job));
    }
    if (m_use_acceleration_structure_uniform_grid)
    {
        RenderJob job;
        job.context = CreateAsyncRenderContext(config, scene_number_one_indexed, AccelerationStructure::UNIFORM_GRID, colour_seed, position_seed);
        m_render_queue.push_back(std::move(job));
    }
    if (m_use_acceleration_structure_hierarchical_uniform_grid)
    {
        RenderJob job;
        job.context = CreateAsyncRenderContext(config, scene_number_one_indexed, AccelerationStructure::HIERARCHICAL_UNIFORM_GRID, colour_seed, position_seed);
        m_render_queue.push_back(std::move(job));
    }
    if (m_use_acceleration_structure_octree)
    {
        RenderJob job;
        job.context = CreateAsyncRenderContext(config, scene_number_one_indexed, AccelerationStructure::OCTREE, colour_seed, position_seed);
        m_render_queue.push_back(std::move(job));
    }
    if (m_use_acceleration_structure_bsp_tree)
    {
        RenderJob job;
        job.context = CreateAsyncRenderContext(config, scene_number_one_indexed, AccelerationStructure::BSP_TREE, colour_seed, position_seed);
        m_render_queue.push_back(std::move(job));
    }
    if (m_use_acceleration_structure_k_d_tree)
    {
        RenderJob job;
        job.context = CreateAsyncRenderContext(config, scene_number_one_indexed, AccelerationStructure::K_D_TREE, colour_seed, position_seed);
        m_render_queue.push_back(std::move(job));
    }
    if (m_use_acceleration_structure_bounding_volume_hierarchy)
    {
        RenderJob job;
        job.context = CreateAsyncRenderContext(config, scene_number_one_indexed, AccelerationStructure::BOUNDING_VOLUME_HIERARCHY, colour_seed, position_seed);
        m_render_queue.push_back(std::move(job));
    }

    if (m_render_queue.empty())
    {
        Logger::Get().LogWarn("No acceleration structures selected");
        return;
    }

    m_render_state = RenderState::RENDERING;
    StartNextRender();
}

void GUIRunner::StartNextRender()
{
    // No more things to render :)
    if (m_current_job_index >= m_render_queue.size())
    {
        return;
    }

    RenderJob& job = m_render_queue[m_current_job_index];

    m_render_thread = std::thread(&GUIRunner::RenderThreadFunc, this);
}

void GUIRunner::CancelRenders()
{
    if (m_current_job_index < m_render_queue.size())
    {
        m_render_queue[m_current_job_index].context.cancel_requested.store(true, std::memory_order_relaxed);
    }

    if (m_render_thread.joinable())
    {
        m_render_thread.join();
    }

    m_render_state = RenderState::IDLE;
    m_render_queue.clear();
    Logger::Get().LogInfo("Render cancelled");
}

void GUIRunner::RenderThreadFunc()
{
    // All done
    if (m_current_job_index >= m_render_queue.size())
    {
        return;
    }

    ExecuteAsyncRender(m_render_queue[m_current_job_index].context);
}

void GUIRunner::Shutdown()
{
    ART::Logger::Get().LogInfo("Shutting down ART [GUI]");

    // Cancel any in-progress render
    if (m_render_state == RenderState::RENDERING)
    {
        CancelRenders();
    }

    if (m_preview_texture)
    {
        SDL_DestroyTexture(m_preview_texture);
        m_preview_texture = nullptr;
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();

    Logger::Get().Flush();
}

} // namespace ART
