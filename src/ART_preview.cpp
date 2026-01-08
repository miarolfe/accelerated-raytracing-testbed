// Copyright Mia Rolfe. All rights reserved.
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

#include "../lib/Camera.h"
#include "../lib/Colour.h"
#include "../lib/Logger.h"
#include "../lib/Material.h"
#include "../lib/RayHittableList.h"
#include "../lib/Sphere.h"
#include "../lib/UniformGrid.h"
#include "../lib/Vec3.h"

using namespace ART;

// SDL_VIDEODRIVER=x11 ./bin/Release/ART
int main(int argc, char* args[])
{
    SDL_Window* window = nullptr;

    static constexpr int window_width = 1280;
    static constexpr int window_height = 720;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return 1;
    }

    window = SDL_CreateWindow
    (
        "ART_preview",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window_width, window_height, SDL_WINDOW_SHOWN
    );

    if (!window)
    {
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer
    (
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    SDL_Texture* texture = SDL_CreateTexture
    (
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    RayHittableList scene;

    Point3 average_position_cluster_1;
    int num_spheres_cluster_1 = 0;

    // Cluster 1
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            for (int k = 0; k < 10; k++)
            {
                const Point3 sphere_position(0.0 + (i * 3.0), 0.0 + (j * 3.0), 0.0 + (k * 3.0));
                average_position_cluster_1 += sphere_position;
                num_spheres_cluster_1++;

                scene.Add
                (
                    std::make_shared<Sphere>
                    (
                        sphere_position, 1.0, std::make_shared<LambertianMaterial>(Colour(RandomCanonicalDouble(), RandomCanonicalDouble(), RandomCanonicalDouble()))
                    )
                );
            }
        }
    }

    assert(num_spheres_cluster_1 > 0);
    average_position_cluster_1 /= num_spheres_cluster_1;

    ART::CameraSetupParams camera_setup_params
    {
        window_width,
        window_height,
        ART::Colour(0.7, 0.8, 1.0),
        18.0,
        10,
        25,
        ART::Point3(-100.0, 100.0, 100.0),
        average_position_cluster_1,
        ART::Vec3(0.0, 1.0, 0.0),
        0.0,
        10.0,
        renderer,
        texture
    };
    ART::Camera camera(camera_setup_params);

    ART::UniformGrid uniform_grid(scene.GetObjects());
    uniform_grid.Create();

    camera.Render(uniform_grid);

    uniform_grid.Destroy();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
