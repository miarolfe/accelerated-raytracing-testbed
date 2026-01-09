// Copyright Mia Rolfe. All rights reserved.

#include "../lib/Camera.h"
#include "../lib/Colour.h"
#include "../lib/HierarchicalUniformGrid.h"
#include "../lib/Logger.h"
#include "../lib/Material.h"
#include "../lib/RayHittableList.h"
#include "../lib/Sphere.h"
#include "../lib/UniformGrid.h"
#include "../lib/Vec3.h"

enum class AccelerationStructure
{
    NONE,
    UNIFORM_GRID,
    HIERARCHICAL_UNIFORM_GRID
};

void RenderWithAccelerationStructure(ART::Camera& camera, ART::RayHittableList& scene, AccelerationStructure acceleration_structure)
{
    switch (acceleration_structure)
    {
        case AccelerationStructure::NONE:
        {
            camera.Render(scene, "render_none.png");
            ART::Logger::Get().LogInfo("Finished render using no acceleration structure");
            break;
        }
        case AccelerationStructure::UNIFORM_GRID:
        {
            ART::UniformGrid uniform_grid(scene.GetObjects());
            camera.Render(uniform_grid, "render_uniform_grid.png");
            ART::Logger::Get().LogInfo("Finished render using uniform grid acceleration structure");
            break;
        }
        case AccelerationStructure::HIERARCHICAL_UNIFORM_GRID:
        {
            ART::HierarchicalUniformGrid hierarchical_uniform_grid(scene.GetObjects());
            camera.Render(hierarchical_uniform_grid, "render_hierarchical_uniform_grid.png");
            ART::Logger::Get().LogInfo("Finished render using hierarchical uniform grid acceleration structure");
            break;
        }
    }
}

void Scene1(AccelerationStructure acceleration_structure)
{
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

    std::shared_ptr<ART::Texture> checker_texture = std::make_shared<ART::CheckerTexture>(0.32, ART::Colour(0.2, 0.3, 0.1), ART::Colour(0.9));
    std::shared_ptr<ART::Material> checker_material = std::make_shared<ART::LambertianMaterial>(checker_texture);
    std::shared_ptr<ART::Material> metal_material = std::make_shared<ART::MetalMaterial>(ART::Colour(0.7), 0.5);
    std::shared_ptr<ART::Material> dielectric_material = std::make_shared<ART::DielectricMaterial>(0.5);
    std::shared_ptr<ART::Material> solid_material = std::make_shared<ART::LambertianMaterial>(std::make_shared<ART::SolidColourTexture>(ART::Colour(0.2, 0.3, 0.2)));

    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(-7.5, 0.0, 10.0), 5.0, checker_material));
    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(0.0, 0.0, 1.0), 0.5, dielectric_material));
    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(7.5, 0.0, 10.0), 2.5, metal_material));

    RenderWithAccelerationStructure(camera, scene, acceleration_structure);
}

void Scene2(AccelerationStructure acceleration_structure)
{
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

    std::shared_ptr<ART::Texture> checker_texture = std::make_shared<ART::CheckerTexture>(
        0.32, ART::Colour(0.2, 0.3, 0.1), ART::Colour(0.9, 0.9, 0.9));

    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(0.0, -10.0, 0.0), 10, std::make_shared<ART::LambertianMaterial>(checker_texture)));
    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(0.0, 10.0, 0.0), 10, std::make_shared<ART::LambertianMaterial>(checker_texture)));

    RenderWithAccelerationStructure(camera, scene, acceleration_structure);
}

void Scene3(AccelerationStructure acceleration_structure)
{
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

    std::shared_ptr<ART::Texture> checker_texture = std::make_shared<ART::CheckerTexture>(
        0.5, ART::Colour(0.0), ART::Colour(1.0));

    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(0.0, -1005.0, 0.0), 1000.0, std::make_shared<ART::LambertianMaterial>(ART::Colour(0.0, 0.8, 0.8))));
    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(0.0, -10.0, 0.0), 10.0, std::make_shared<ART::LambertianMaterial>(checker_texture)));
    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(0.0, 1.0, 0.0), 1.0, std::make_shared<ART::DielectricMaterial>(1.5)));
    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(0.0, 1.0, 0.0), 0.8, std::make_shared<ART::DielectricMaterial>(1.0 / 1.5)));

    RenderWithAccelerationStructure(camera, scene, acceleration_structure);
}

void Scene4(AccelerationStructure acceleration_structure)
{
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

    std::shared_ptr<ART::Texture> checker_texture = std::make_shared<ART::CheckerTexture>(
        0.5, ART::Colour(0.0), ART::Colour(1.0));

    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(0.0, -10.0, 0.0), 10.0, std::make_shared<ART::LambertianMaterial>(checker_texture)));
    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(0.0, 1.0, 0.0), 1.0, std::make_shared<ART::DielectricMaterial>(1.5)));
    scene.Add(std::make_shared<ART::Sphere>(ART::Point3(0.0, 1.0, 0.0), 0.8, std::make_shared<ART::DielectricMaterial>(1.0 / 1.5)));

    ART::UniformGrid uniform_grid(scene.GetObjects());

    RenderWithAccelerationStructure(camera, scene, acceleration_structure);
}

void Scene5(AccelerationStructure acceleration_structure)
{
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

                scene.Add
                (
                    std::make_shared<ART::Sphere>
                    (
                        sphere_position, 1.0, std::make_shared<ART::LambertianMaterial>(ART::Colour(ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble()))
                    )
                );
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
                scene.Add
                (
                    std::make_shared<ART::Sphere>
                    (
                        sphere_position, 1.0, std::make_shared<ART::LambertianMaterial>(ART::Colour(ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble()))
                    )
                );
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
                scene.Add
                (
                    std::make_shared<ART::Sphere>
                    (
                        sphere_position, 1.0, std::make_shared<ART::LambertianMaterial>(ART::Colour(ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble(), ART::RandomCanonicalDouble()))
                    )
                );
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
        200,
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

    Scene5(AccelerationStructure::NONE);
    Scene5(AccelerationStructure::UNIFORM_GRID);
    Scene5(AccelerationStructure::HIERARCHICAL_UNIFORM_GRID);

    ART::Logger::Get().LogInfo("Shutting down");
    ART::Logger::Get().Flush();
}
