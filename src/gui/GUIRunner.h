// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <SDL3/SDL.h>

namespace ART
{

class GUIRunner
{
public:
    GUIRunner();
    ~GUIRunner();

    void Init();
    void Shutdown();

private:
    void Run();

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool m_running = false;
    int m_window_width = 1280;
    int m_window_height = 720;
};

} // namespace ART
