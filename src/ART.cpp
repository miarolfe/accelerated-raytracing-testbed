// Copyright Mia Rolfe. All rights reserved.

#include <iomanip>

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

ART::RenderStats RenderWithAccelerationStructure(ART::Camera& camera, ART::RayHittableList& scene, ART::AccelerationStructure acceleration_structure)
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
            camera.Render(scene, "render_none.png");
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
            camera.Render(uniform_grid, "render_uniform_grid.png");
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
            camera.Render(hierarchical_uniform_grid, "render_hierarchical_uniform_grid.png");
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
            camera.Render(bounding_volume_hierarchy, "render_bounding_volume_hierarchy.png");
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
    }

    LogRenderStats(stats);
    return stats;
}

void Scene1(ART::AccelerationStructure acceleration_structure)
{
    ART::ArenaAllocator arena(ART::ONE_MEGABYTE);

    ART::CameraSetupParams camera_setup_params
    {
        600,                            // image_width
        400,                            // image_height
        ART::Colour(0.7, 0.8, 1.0),     // background_colour
        90.0,                           // vertical_fov
        100,                            // samples_per_pixel
        25,                             // max_ray_bounces
        ART::Point3(0, 0.0, 0.0),       // look_from
        ART::Point3(0.0, 0.0, 10.0),    // look_at
        ART::Vec3(0.0, 1.0, 0.0),       // up
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    ART::Camera camera(camera_setup_params);

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

    RenderWithAccelerationStructure(camera, scene, acceleration_structure);
}

void Scene2(ART::AccelerationStructure acceleration_structure)
{
    ART::ArenaAllocator arena(ART::ONE_MEGABYTE);

    ART::CameraSetupParams camera_setup_params
    {
        1280,                           // image_width
        720,                            // image_height
        ART::Colour(0.7, 0.8, 1.0),     // background_colour
        20.0,                           // vertical_fov
        100,                            // samples_per_pixel
        25,                             // max_ray_bounces
        ART::Point3(13.0, 2.0, 3.0),    // look_from
        ART::Point3(0.0, 0.0, 0.0),     // look_at
        ART::Vec3(0.0, 1.0, 0.0),       // up
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    ART::Camera camera(camera_setup_params);

    ART::RayHittableList scene;

    ART::Texture* even_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(0.2, 0.3, 0.1));
    ART::Texture* odd_texture = arena.Create<ART::SolidColourTexture>(ART::Colour(0.9, 0.9, 0.9));
    ART::Texture* checker_texture = arena.Create<ART::CheckerTexture>(0.32, even_texture, odd_texture);
    ART::Material* checker_material = arena.Create<ART::LambertianMaterial>(checker_texture);

    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, -10.0, 0.0), 10.0, checker_material));
    scene.Add(arena.Create<ART::Sphere>(ART::Point3(0.0, 10.0, 0.0), 10.0, checker_material));

    RenderWithAccelerationStructure(camera, scene, acceleration_structure);
}

void Scene3(ART::AccelerationStructure acceleration_structure)
{
    ART::ArenaAllocator arena(ART::ONE_MEGABYTE); // 1 MB

    ART::CameraSetupParams camera_setup_params
    {
        1920,                           // image_width
        1080,                           // image_height
        ART::Colour(0.7, 0.8, 1.0),     // background_colour
        20.0,                           // vertical_fov
        200,                            // samples_per_pixel
        50,                             // max_ray_bounces
        ART::Point3(13.0, 2.0, 3.0),    // look_from
        ART::Point3(0.0, 0.0, 0.0),     // look_at
        ART::Vec3(0.0, 1.0, 0.0),       // up
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    ART::Camera camera(camera_setup_params);

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

    RenderWithAccelerationStructure(camera, scene, acceleration_structure);
}

void Scene4(ART::AccelerationStructure acceleration_structure)
{
    ART::ArenaAllocator arena(ART::ONE_MEGABYTE);

    ART::CameraSetupParams camera_setup_params
    {
        600,                            // image_width
        400,                            // image_height
        ART::Colour(0.7, 0.8, 1.0),     // background_colour
        20.0,                           // vertical_fov
        100,                            // samples_per_pixel
        25,                             // max_ray_bounces
        ART::Point3(13.0, 2.0, 3.0),    // look_from
        ART::Point3(0.0, 0.0, 0.0),     // look_at
        ART::Vec3(0.0, 1.0, 0.0),       // up
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    ART::Camera camera(camera_setup_params);

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

    ART::UniformGrid uniform_grid(scene.GetObjects());

    RenderWithAccelerationStructure(camera, scene, acceleration_structure);
}

void Scene5(ART::AccelerationStructure acceleration_structure)
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

    ART::CameraSetupParams camera_setup_params
    {
        1280,
        720,
        ART::Colour(0.7, 0.8, 1.0),
        18.0,
        1,
        25,
        ART::Point3(-100.0, 100.0, 100.0),
        average_position_cluster_1,
        ART::Vec3(0.0, 1.0, 0.0),
        0.0,
        10.0
    };
    ART::Camera camera(camera_setup_params);

    RenderWithAccelerationStructure(camera, scene, acceleration_structure);
}

int main()
{
    ART::Logger::Get().LogInfo("Booting up");

    Scene5(ART::AccelerationStructure::NONE);
    Scene5(ART::AccelerationStructure::UNIFORM_GRID);
    Scene5(ART::AccelerationStructure::HIERARCHICAL_UNIFORM_GRID);
    Scene5(ART::AccelerationStructure::BOUNDING_VOLUME_HIERARCHY);

    ART::Logger::Get().LogInfo("Shutting down");
    ART::Logger::Get().Flush();
}
