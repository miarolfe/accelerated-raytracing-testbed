// Copyright Mia Rolfe. All rights reserved.

#include <KDTree.h>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>

#include <ArenaAllocator.h>
#include <BoundingVolumeHierarchy.h>
#include <Camera.h>
#include <Colour.h>
#include <HierarchicalUniformGrid.h>
#include <Logger.h>
#include <Material.h>
#include <RayHittableList.h>
#include <Sphere.h>
#include <Texture.h>
#include <Timer.h>
#include <UniformGrid.h>
#include <Utility.h>
#include <Vec3.h>

struct CLIParams
{
public:
    std::size_t screen_width = 1280;
    std::size_t screen_height = 720;
    std::size_t samples_per_pixel = 100;
    int scene = 5;
};

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
            if (out_params.scene < 1 || out_params.scene > 5)
            {
                std::cerr << "Error: --scene must be between 1 and 5\n";
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

ART::CameraRenderConfig MakeCameraRenderConfig(const CLIParams& cli_params)
{
    return ART::CameraRenderConfig
    {
        cli_params.screen_width,
        cli_params.screen_height,
        cli_params.samples_per_pixel,
        25
    };
}

void LogRenderConfig(const ART::CameraRenderConfig& render_config, int scene_number)
{
    std::ostringstream output_string_stream;
    output_string_stream << "Configuration: " << render_config.image_width << "x" << render_config.image_height
                         << ", " << render_config.samples_per_pixel << " samples per pixel";
    ART::Logger::Get().LogInfo(output_string_stream.str());
}

void LogRenderStats(const ART::RenderStats& stats)
{
    std::ostringstream output_string_stream;
    // Use fixed-point (no scientific notation)
    output_string_stream << std::fixed << std::setprecision(2);
    output_string_stream << "[Acceleration structure: " << ART::AccelerationStructureToString(stats.m_acceleration_structure) << "] "
        << "Construction time: " << stats.m_construction_time_ms << " ms, "
        << "Render time: " << stats.m_render_time_ms << " ms, "
        << "Total time: " << stats.TotalTimeMilliseconds() << " ms";
    ART::Logger::Get().LogInfo(output_string_stream.str());
}

ART::RenderStats RenderWithAccelerationStructure(ART::Camera& camera, ART::RayHittableList& scene, const ART::SceneConfig& scene_config, ART::AccelerationStructure acceleration_structure)
{
    ART::Timer timer;
    ART::RenderStats stats;
    stats.m_acceleration_structure = acceleration_structure;

    switch (acceleration_structure)
    {
        case ART::AccelerationStructure::NONE:
        {
            stats.m_construction_time_ms = 0.0;

            timer.Start();
            camera.Render(scene, scene_config, "render_none.png");
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
        case ART::AccelerationStructure::UNIFORM_GRID:
        {
            timer.Start();
            ART::UniformGrid uniform_grid(scene.GetObjects());
            timer.Stop();
            stats.m_construction_time_ms = timer.ElapsedMilliseconds();

            timer.Start();
            camera.Render(uniform_grid, scene_config, "render_uniform_grid.png");
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
        case ART::AccelerationStructure::HIERARCHICAL_UNIFORM_GRID:
        {
            timer.Start();
            ART::HierarchicalUniformGrid hierarchical_uniform_grid(scene.GetObjects());
            timer.Stop();
            stats.m_construction_time_ms = timer.ElapsedMilliseconds();

            timer.Start();
            camera.Render(hierarchical_uniform_grid, scene_config, "render_hierarchical_uniform_grid.png");
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
        case ART::AccelerationStructure::K_D_TREE:
        {
            timer.Start();
            ART::KDTreeNode hierarchical_uniform_grid(scene.GetObjects());
            timer.Stop();
            stats.m_construction_time_ms = timer.ElapsedMilliseconds();

            timer.Start();
            camera.Render(hierarchical_uniform_grid, scene_config, "render_k_d_tree.png");
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
        case ART::AccelerationStructure::BOUNDING_VOLUME_HIERARCHY:
        {
            timer.Start();
            ART::BVHNode bounding_volume_hierarchy(scene.GetObjects());
            timer.Stop();
            stats.m_construction_time_ms = timer.ElapsedMilliseconds();

            timer.Start();
            camera.Render(bounding_volume_hierarchy, scene_config, "render_bounding_volume_hierarchy.png");
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
    }

    LogRenderStats(stats);
    return stats;
}

void Scene1(const ART::CameraRenderConfig& render_config, ART::AccelerationStructure acceleration_structure)
{
    ART::ArenaAllocator arena(ART::ONE_MEGABYTE);

    ART::CameraViewConfig view_config
    {
        ART::Point3(0, 0.0, 0.0),       // look_from
        ART::Point3(0.0, 0.0, 10.0),    // look_at
        ART::Vec3(0.0, 1.0, 0.0),       // up
        90.0,                           // vertical_fov
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    ART::Camera camera(view_config, render_config);

    ART::SceneConfig scene_config
    {
        ART::Colour(0.7, 0.8, 1.0)      // background_colour
    };

    ART::RayHittableList scene;

    ART::Texture* even_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(0.2, 0.3, 0.1));
    ART::Texture* odd_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(0.9, 0.9, 0.9));
    ART::Texture* checker_texture = arena.Create<ART::CheckerTexture>(0.32, even_texture, odd_texture);
    ART::Material* checker_material = arena.Create<ART::LambertianMaterial>(checker_texture);
    ART::Material* metal_material = arena.Create<ART::MetalMaterial>(ART::Colour(0.7), 0.5);
    ART::Material* dielectric_material = arena.Create<ART::DielectricMaterial>(0.5);

    scene.Add(arena.Create<ART::Sphere>(ART::Point3(-7.5, 0.0, 10.0), 5.0, checker_material));
    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, 0.0, 1.0), 0.5, dielectric_material));
    scene.Add(arena.Create<ART::Sphere>(ART::Point3(7.5, 0.0, 10.0), 2.5, metal_material));

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

void Scene2(const ART::CameraRenderConfig& render_config, ART::AccelerationStructure acceleration_structure)
{
    ART::ArenaAllocator arena(ART::ONE_MEGABYTE);

    ART::CameraViewConfig view_config
    {
        ART::Point3(13.0, 2.0, 3.0),    // look_from
        ART::Point3(0.0, 0.0, 0.0),     // look_at
        ART::Vec3(0.0, 1.0, 0.0),       // up
        20.0,                           // vertical_fov
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    ART::Camera camera(view_config, render_config);

    ART::SceneConfig scene_config
    {
        ART::Colour(0.7, 0.8, 1.0)      // background_colour
    };

    ART::RayHittableList scene;

    ART::Texture* even_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(0.2, 0.3, 0.1));
    ART::Texture* odd_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(0.9, 0.9, 0.9));
    ART::Texture* checker_texture = arena.Create<ART::CheckerTexture>(0.32, even_texture, odd_texture);
    ART::Material* checker_material = arena.Create<ART::LambertianMaterial>(checker_texture);

    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, -10.0, 0.0), 10.0, checker_material));
    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, 10.0, 0.0), 10.0, checker_material));

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

void Scene3(const ART::CameraRenderConfig& render_config, ART::AccelerationStructure acceleration_structure)
{
    ART::ArenaAllocator arena(ART::ONE_MEGABYTE);

    // Scene 3 uses more ray bounces for glass materials
    ART::CameraRenderConfig scene3_render_config = render_config;
    scene3_render_config.max_ray_bounces = 50;

    ART::CameraViewConfig view_config
    {
        ART::Point3(13.0, 2.0, 3.0),    // look_from
        ART::Point3(0.0, 0.0, 0.0),     // look_at
        ART::Vec3(0.0, 1.0, 0.0),       // up
        20.0,                           // vertical_fov
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    ART::Camera camera(view_config, scene3_render_config);

    ART::SceneConfig scene_config
    {
        ART::Colour(0.7, 0.8, 1.0)      // background_colour
    };

    ART::RayHittableList scene;

    ART::Texture* even_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(0.0, 0.0, 0.0));
    ART::Texture* odd_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(1.0, 1.0, 1.0));
    ART::Texture* checker_texture = arena.Create<ART::CheckerTexture>(0.5, even_texture, odd_texture);

    ART::Texture* cyan_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(0.0, 0.8, 0.8));
    ART::Material* cyan_material = arena.Create<ART::LambertianMaterial>(cyan_texture);
    ART::Material* checker_material = arena.Create<ART::LambertianMaterial>(checker_texture);
    ART::Material* glass_material = arena.Create<ART::DielectricMaterial>(1.5);
    ART::Material* glass_inner_material = arena.Create<ART::DielectricMaterial>(1.0 / 1.5);

    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, -1005.0, 0.0), 1000.0, cyan_material));
    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, -10.0, 0.0), 10.0, checker_material));
    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, 1.0, 0.0), 1.0, glass_material));
    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, 1.0, 0.0), 0.8, glass_inner_material));

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

void Scene4(const ART::CameraRenderConfig& render_config, ART::AccelerationStructure acceleration_structure)
{
    ART::ArenaAllocator arena(ART::ONE_MEGABYTE);

    ART::CameraViewConfig view_config
    {
        ART::Point3(13.0, 2.0, 3.0),    // look_from
        ART::Point3(0.0, 0.0, 0.0),     // look_at
        ART::Vec3(0.0, 1.0, 0.0),       // up
        20.0,                           // vertical_fov
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    ART::Camera camera(view_config, render_config);

    ART::SceneConfig scene_config
    {
        ART::Colour(0.7, 0.8, 1.0)      // background_colour
    };

    ART::RayHittableList scene;

    ART::Texture* even_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(0.0, 0.0, 0.0));
    ART::Texture* odd_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(1.0, 1.0, 1.0));
    ART::Texture* checker_texture = arena.Create<ART::CheckerTexture>(0.5, even_texture, odd_texture);
    ART::Material* checker_material = arena.Create<ART::LambertianMaterial>(checker_texture);
    ART::Material* glass_material = arena.Create<ART::DielectricMaterial>(1.5);
    ART::Material* glass_inner_material = arena.Create<ART::DielectricMaterial>(1.0 / 1.5);

    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, -10.0, 0.0), 10.0, checker_material));
    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, 1.0, 0.0), 1.0, glass_material));
    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, 1.0, 0.0), 0.8, glass_inner_material));

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

void Scene5(const ART::CameraRenderConfig& render_config, ART::AccelerationStructure acceleration_structure)
{
    ART::ArenaAllocator arena(ART::ONE_MEGABYTE);

    ART::RayHittableList scene;

    ART::Point3 average_position_cluster_1;
    int num_spheres_cluster_1 = 0;

    // Cluster 1
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            for (int k = 0; k < 10; k++)
            {
                const ART::Point3 sphere_position(0.0 + (i * 3.0), 0.0 + (j * 3.0), 0.0 + (k * 3.0));
                average_position_cluster_1 += sphere_position;
                num_spheres_cluster_1++;

                ART::Texture* texture = arena.Create<ART::SolidColourTexture>(ART::Colour(ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble()));
                ART::Material* material = arena.Create<ART::LambertianMaterial>(texture);
                scene.Add(arena.Create<ART::Sphere>(sphere_position, 1.0, material));
            }
        }
    }

    // Cluster 2
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            for (int k = 0; k < 10; k++)
            {
                const ART::Point3 sphere_position(500.0 + (i * 3.0), 500.0 + (j * 3.0), 500.0 + (k * 3.0));
                ART::Texture* texture = arena.Create<ART::SolidColourTexture>(ART::Colour(ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble()));
                ART::Material* material = arena.Create<ART::LambertianMaterial>(texture);
                scene.Add(arena.Create<ART::Sphere>(sphere_position, 1.0, material));
            }
        }
    }

    // Cluster 3
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            for (int k = 0; k < 10; k++)
            {
                const ART::Point3 sphere_position(1000.0 + (i * 3.0), -500.0 + (j * 3.0), 1000.0 + (k * 3.0));
                ART::Texture* texture = arena.Create<ART::SolidColourTexture>(ART::Colour(ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble()));
                ART::Material* material = arena.Create<ART::LambertianMaterial>(texture);
                scene.Add(arena.Create<ART::Sphere>(sphere_position, 1.0, material));
            }
        }
    }

    assert(num_spheres_cluster_1 > 0);
    average_position_cluster_1 /= num_spheres_cluster_1;

    ART::CameraViewConfig view_config
    {
        ART::Point3(-100.0, 100.0, 100.0),  // look_from
        average_position_cluster_1,          // look_at
        ART::Vec3(0.0, 1.0, 0.0),            // up
        18.0,                                // vertical_fov
        0.0,                                 // defocus_angle
        10.0                                 // focus_distance
    };
    ART::Camera camera(view_config, render_config);

    ART::SceneConfig scene_config
    {
        ART::Colour(0.7, 0.8, 1.0)      // background_colour
    };

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

void RenderScene(const ART::CameraRenderConfig& render_config, int scene_number, ART::AccelerationStructure acceleration_structure)
{
    switch (scene_number)
    {
        case 1: Scene1(render_config, acceleration_structure); break;
        case 2: Scene2(render_config, acceleration_structure); break;
        case 3: Scene3(render_config, acceleration_structure); break;
        case 4: Scene4(render_config, acceleration_structure); break;
        case 5: Scene5(render_config, acceleration_structure); break;
    }
}

int main(int argc, char* argv[])
{
    CLIParams cli_params;
    if (!ParseCLIArgs(argc, argv, cli_params))
    {
        return 1;
    }

    ART::Logger::Get().LogInfo("Booting up");

    ART::CameraRenderConfig render_config = MakeCameraRenderConfig(cli_params);
    LogRenderConfig(render_config, cli_params.scene);

    RenderScene(render_config, cli_params.scene, ART::AccelerationStructure::NONE);
    RenderScene(render_config, cli_params.scene, ART::AccelerationStructure::UNIFORM_GRID);
    RenderScene(render_config, cli_params.scene, ART::AccelerationStructure::HIERARCHICAL_UNIFORM_GRID);
    RenderScene(render_config, cli_params.scene, ART::AccelerationStructure::K_D_TREE);
    RenderScene(render_config, cli_params.scene, ART::AccelerationStructure::BOUNDING_VOLUME_HIERARCHY);

    ART::Logger::Get().LogInfo("Shutting down");
    ART::Logger::Get().Flush();

    return 0;
}
