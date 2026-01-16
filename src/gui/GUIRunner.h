// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Camera.h>
#include <Utility.h>

// Fwd decls
struct SDL_Window;
struct SDL_Renderer;

namespace ART
{

class GUIRunner
{
public:
    void Init();
    void Shutdown();

private:
    void DrawUI();
    void CloseUI();
    void RunRender();

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool m_running = false;
    bool m_render_requested = false;

    int m_render_width = 1280;
    int m_render_height = 720;
    int m_samples_per_pixel = 100;

    // 0-indexed
    int m_scene_number = 5;

    bool m_use_acceleration_structure_none = true;
    bool m_use_acceleration_structure_uniform_grid = true;
    bool m_use_acceleration_structure_hierarchical_uniform_grid = true;
    bool m_use_acceleration_structure_octree = true;
    bool m_use_acceleration_structure_bsp_tree = true;
    bool m_use_acceleration_structure_k_d_tree = true;
    bool m_use_acceleration_structure_bounding_volume_hierarchy = true;
};

} // namespace ART
