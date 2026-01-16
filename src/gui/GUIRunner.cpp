// Copyright Mia Rolfe. All rights reserved.
#include <GUIRunner.h>

#include <SDL3/SDL.h>
#include <string>

#include <Logger.h>

namespace ART
{

GUIRunner::GUIRunner()
{

}

GUIRunner::~GUIRunner()
{

}

void GUIRunner::Init()
{
    Logger::Get().LogInfo("Booting up");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        Logger::Get().LogFatal(std::string("Failed to initialize SDL: ") + SDL_GetError());
        return;
    }

    m_window = SDL_CreateWindow
    (
        "ART - Accelerated Raytracing Testbed",
        m_window_width,
        m_window_height,
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

    m_running = true;
    Run();
}

void GUIRunner::Run()
{
    while (m_running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                m_running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                {
                    m_running = false;
                }
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                m_window_width = event.window.data1;
                m_window_height = event.window.data2;
                break;
            }
        }

        SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
        SDL_RenderClear(m_renderer);

        SDL_RenderPresent(m_renderer);
    }
}

void GUIRunner::Shutdown()
{
    Logger::Get().LogInfo("Shutting down");

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
