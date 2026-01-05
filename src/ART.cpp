// Copyright Mia Rolfe. All rights reserved.
#include <memory>
#include "../lib/Camera.h"
#include "../lib/Colour.h"
#include "../lib/Logger.h"
#include "../lib/Material.h"
#include "../lib/RayHittableList.h"
#include "../lib/Sphere.h"
#include "../lib/Vec3.h"

int main()
{
    ART::Logger::Get().LogInfo("Booting up");

    ART::CameraSetupParams camera_setup_params
    {
        600,                                // image_width
        400,                                // image_height
        3,                                  // num_image_components
        ART::Colour(0.7, 0.8, 1.0),         // background_colour
        90.0,                               // vertical_fov
        500,                                 // samples_per_pixel
        25,                                 // max_ray_bounces
        ART::Point3(0, 0.0, 0.0),        // look_from
        ART::Point3(0.0, 0.0, 10.0),        // look_at
        ART::Vec3(0.0, 1.0, 0.0),           // up
        0.0,                                // defocus_angle
        10.0                                // focus_distance
    };
    ART::Camera camera(camera_setup_params);

    ART::RayHittableList world;

    std::shared_ptr<ART::Texture> checker_texture = std::make_shared<ART::CheckerTexture>(0.32, ART::Colour(0.2, 0.3, 0.1), ART::Colour(0.9));
    // std::shared_ptr<ART::Texture> checker_texture = std::make_shared<ART::SolidColourTexture>(ART::Colour(1.0, 0.0, 0.0));
    std::shared_ptr<ART::Material> checker_material = std::make_shared<ART::LambertianMaterial>(checker_texture);

    world.Add(std::make_shared<ART::Sphere>(ART::Point3(-7.5, 0.0, 10.0), 5.0, checker_material));
    world.Add(std::make_shared<ART::Sphere>(ART::Point3(7.5, 0.0, 10.0), 5.0, checker_material));

    camera.Render(world);

    ART::Logger::Get().Flush();
}
