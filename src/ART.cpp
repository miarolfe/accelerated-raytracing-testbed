// Copyright Mia Rolfe. All rights reserved.
#include <memory>
#include "../lib/Camera.h"
#include "../lib/Colour.h"
#include "../lib/Logger.h"
#include "../lib/Material.h"
#include "../lib/RayHittableList.h"
#include "../lib/Sphere.h"
#include "../lib/UniformGrid.h"
#include "../lib/Vec3.h"

void Scene1()
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

    camera.Render(scene);
}

void Scene2()
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

    camera.Render(scene);
}

void Scene3()
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

    camera.Render(scene);
}

void Scene4()
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
    uniform_grid.Create();

    camera.Render(uniform_grid);

    uniform_grid.Destroy();
}

int main()
{
    ART::Logger::Get().LogInfo("Booting up");

    Scene4();

    ART::Logger::Get().Flush();
}
