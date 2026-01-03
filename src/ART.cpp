// Copyright Mia Rolfe. All rights reserved.
#include <memory>
#include "../lib/Camera.h"
#include "../lib/Logger.h"
#include "../lib/Material.h"
#include "../lib/Sphere.h"
#include "../lib/Vec3.h"

int main()
{
    ART::Logger::Get().LogInfo("Booting up");

    ART::Camera camera;

    std::shared_ptr<ART::Material> ground_material = std::make_shared<ART::LambertianMaterial>(ART::Colour(0.0, 0.0, 0.0));
    ART::Sphere world(ART::Point3(0.0, 0.0, 10.0), 1, ground_material);

    camera.Render(world);
}
