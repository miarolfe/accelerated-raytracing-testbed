// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include <Common/RenderCommon.h>
#include <Core/Utility.h>
#include <RayTracing/Camera.h>

// Fwd decls
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace ART
{

// Represents a single render job in the queue
struct RenderJob
{
public:
    RenderContext context;
};

enum class RenderState
{
    IDLE,
    RENDERING,
    COMPLETED
};

class GUIRunner
{
public:
    void Init();

    void Shutdown();

private:
    void DrawUI();

    void DrawSettingsUI();

    void DrawRenderingUI();

    static std::string FormatMemoryUsed(std::size_t bytes);

    void DrawResultsUI();

    void UpdateRenderState();

    void UpdatePreviewTexture();

    void StartRenderQueue();

    void StartNextRender();

    void CancelRenders();

    void RenderThreadFunc();

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool m_running = false;

    bool m_use_acceleration_structure_none = true;
    bool m_use_acceleration_structure_uniform_grid = true;
    bool m_use_acceleration_structure_hierarchical_uniform_grid = true;
    bool m_use_acceleration_structure_octree = true;
    bool m_use_acceleration_structure_bsp_tree = true;
    bool m_use_acceleration_structure_k_d_tree = true;
    bool m_use_acceleration_structure_bounding_volume_hierarchy = true;

    int m_render_width = 1280;
    int m_render_height = 720;
    int m_samples_per_pixel = 100;
    int m_scene_number = 5; // 0-indexed

    RenderState m_render_state = RenderState::IDLE;
    std::vector<RenderJob> m_render_queue;
    std::size_t m_current_job_index = 0;
    std::thread m_render_thread; // UI on main thread

    SDL_Texture* m_preview_texture = nullptr;
    int m_preview_width = 0;
    int m_preview_height = 0;

    std::vector<RenderStats> m_completed_stats;
};

} // namespace ART
