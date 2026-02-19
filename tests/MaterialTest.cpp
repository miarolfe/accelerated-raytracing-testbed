// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Materials/Material.h>
#include <Materials/Texture.h>
#include <Maths/Ray.h>
#include <Maths/Vec3.h>
#include <RayTracing/RayHitResult.h>

namespace ART
{

// Utility helper for tests.
// Constructs a RayHitResult with a known front-facing surface at the origin,
// its normal pointing in +z.
static RayHitResult MakeFrontFacingResult(const Point3& point, const Vec3& outward_normal)
{
    RayHitResult result;
    result.m_point = point;
    result.m_t = 1.0;
    result.m_u = 0.5;
    result.m_v = 0.5;
    result.m_material = nullptr;

    // Ray coming from +z toward the surface sets dot < 0, so front-facing
    const Ray incoming_ray(point + outward_normal, -outward_normal);
    result.SetFaceNormal(incoming_ray, outward_normal);

    return result;
}

TEST_CASE("Base Material Scatter always returns false", "[Material]")
{
    // Base material shouldn't do anything, really
    const Material material;
    const Ray ray(Point3(0.0, 0.0, 1.0), Vec3(0.0, 0.0, -1.0));
    const RayHitResult result = MakeFrontFacingResult(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0));

    Colour attenuation;
    Ray out_ray;
    REQUIRE(material.Scatter(ray, result, attenuation, out_ray) == false);
}

TEST_CASE("Base Material Emitted returns black", "[Material]")
{
    // Base material shouldn't do anything, really
    const Material material;
    const Colour emitted = material.Emitted(0.5, 0.5, Point3(0.0, 0.0, 0.0));

    REQUIRE(emitted.m_x == Approx(0.0));
    REQUIRE(emitted.m_y == Approx(0.0));
    REQUIRE(emitted.m_z == Approx(0.0));
}

TEST_CASE("LambertianMaterial Scatter returns true and sets attenuation from texture", "[Material]")
{
    SolidColourTexture texture(Colour(0.1, 0.2, 0.3));
    const LambertianMaterial material(&texture);

    const Ray ray(Point3(0.0, 0.0, 1.0), Vec3(0.0, 0.0, -1.0));
    const RayHitResult result = MakeFrontFacingResult(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0));

    Colour attenuation;
    Ray out_ray;
    const bool scattered = material.Scatter(ray, result, attenuation, out_ray);

    REQUIRE(scattered == true);
    REQUIRE(attenuation.m_x == Approx(0.1));
    REQUIRE(attenuation.m_y == Approx(0.2));
    REQUIRE(attenuation.m_z == Approx(0.3));
}

TEST_CASE("LambertianMaterial Scatter ray should have origin at hit point", "[Material]")
{
    SolidColourTexture texture(Colour(0.5));
    const LambertianMaterial material(&texture);

    const Point3 hit_point(1.0, 2.0, 3.0);
    const Ray ray(Point3(1.0, 2.0, 13.0), Vec3(0.0, 0.0, -1.0));
    const RayHitResult result = MakeFrontFacingResult(hit_point, Vec3(0.0, 0.0, 1.0));

    Colour attenuation;
    Ray out_ray;
    material.Scatter(ray, result, attenuation, out_ray);

    REQUIRE(out_ray.m_origin.m_x == Approx(hit_point.m_x));
    REQUIRE(out_ray.m_origin.m_y == Approx(hit_point.m_y));
    REQUIRE(out_ray.m_origin.m_z == Approx(hit_point.m_z));
}

TEST_CASE("LambertianMaterial Scatter direction is on the correct hemisphere", "[Material]")
{
    SolidColourTexture texture(Colour(0.5));
    const LambertianMaterial material(&texture);

    const Vec3 normal(0.0, 1.0, 0.0); // surface pointing up
    const RayHitResult result = MakeFrontFacingResult(Point3(0.0, 0.0, 0.0), normal);
    const Ray ray(Point3(0.0, 1.0, 0.0), Vec3(0.0, -1.0, 0.0));

    // Scattered direction must be on the same side as the normal (upper hemisphere)
    bool scattered_direction_always_on_same_side_as_normal = true;

    // Run several times since direction is random
    for (int i = 0; i < 10000; i++)
    {
        Colour attenuation;
        Ray out_ray;
        material.Scatter(ray, result, attenuation, out_ray);

        double dot = Dot(out_ray.m_direction, normal);
        if (dot < 0.0)
        {
            scattered_direction_always_on_same_side_as_normal = false;
        }
    }

    REQUIRE(scattered_direction_always_on_same_side_as_normal);
}

TEST_CASE("MetalMaterial Scatter reflects ray correctly when fuzz is zero", "[Material]")
{
    const MetalMaterial material(Colour(0.9, 0.9, 0.9), 0.0);

    // Ray going straight down (-y), surface normal pointing up (+y)
    const Ray ray(Point3(0.0, 1.0, 0.0), Vec3(0.0, -1.0, 0.0));
    const RayHitResult result = MakeFrontFacingResult(Point3(0.0, 0.0, 0.0), Vec3(0.0, 1.0, 0.0));

    Colour attenuation;
    Ray out_ray;
    const bool scattered = material.Scatter(ray, result, attenuation, out_ray);

    const double fp_epsilon = 1e-10;

    REQUIRE(scattered == true);
    // Reflected ray should go upward (+y), with zero x and z components
    REQUIRE(out_ray.m_direction.m_y > 0.0);
    REQUIRE(out_ray.m_direction.m_x == Approx(0.0).margin(fp_epsilon));
    REQUIRE(out_ray.m_direction.m_z == Approx(0.0).margin(fp_epsilon));
}

TEST_CASE("MetalMaterial Scatter sets attenuation to albedo", "[Material]")
{
    const MetalMaterial material(Colour(0.1, 0.2, 0.3), 0.0);

    const Ray ray(Point3(0.0, 1.0, 0.0), Vec3(0.0, -1.0, 0.0));
    const RayHitResult result = MakeFrontFacingResult(Point3(0.0, 0.0, 0.0), Vec3(0.0, 1.0, 0.0));

    Colour attenuation;
    Ray out_ray;
    material.Scatter(ray, result, attenuation, out_ray);

    REQUIRE(attenuation.m_x == Approx(0.1));
    REQUIRE(attenuation.m_y == Approx(0.2));
    REQUIRE(attenuation.m_z == Approx(0.3));
}

TEST_CASE("DielectricMaterial Scatter always returns true", "[Material]")
{
    const DielectricMaterial material(1.5);

    const Ray ray(Point3(0.0, 0.0, 1.0), Vec3(0.0, 0.0, -1.0));
    const RayHitResult result = MakeFrontFacingResult(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0));

    Colour attenuation;
    Ray out_ray;
    REQUIRE(material.Scatter(ray, result, attenuation, out_ray) == true);
}

TEST_CASE("DielectricMaterial Scatter always produces white attenuation", "[Material]")
{
    const DielectricMaterial material(1.5);

    const Ray ray(Point3(0.0, 0.0, 1.0), Vec3(0.0, 0.0, -1.0));
    const RayHitResult result = MakeFrontFacingResult(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0));

    bool dielectric_attenuation_is_always_white = true;

    // Run several times since reflect vs refract is probabilistic (Schlick)
    for (int i = 0; i < 10000; i++)
    {
        Colour attenuation;
        Ray out_ray;
        material.Scatter(ray, result, attenuation, out_ray);

        const bool dielectric_attenuation_is_white =
            (attenuation.m_x == Approx(1.0)) &&
            (attenuation.m_y == Approx(1.0)) &&
            (attenuation.m_z == Approx(1.0));

        if (!dielectric_attenuation_is_white)
        {
            dielectric_attenuation_is_always_white = false;
        }
    }

    REQUIRE(dielectric_attenuation_is_always_white);
}

TEST_CASE("DielectricMaterial Scatter produces non-NaN valid direction", "[Material]")
{
    const DielectricMaterial material(1.5);

    const Ray ray(Point3(0.0, 0.0, 1.0), Vec3(0.0, 0.0, -1.0));
    const RayHitResult result = MakeFrontFacingResult(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0));

    Colour attenuation;
    Ray out_ray;
    material.Scatter(ray, result, attenuation, out_ray);

    REQUIRE(!std::isnan(out_ray.m_direction.m_x));
    REQUIRE(!std::isnan(out_ray.m_direction.m_y));
    REQUIRE(!std::isnan(out_ray.m_direction.m_z));
}

TEST_CASE("DiffuseLightMaterial Emitted returns texture value", "[Material]")
{
    SolidColourTexture texture(Colour(0.1, 0.2, 0.3));
    const DiffuseLightMaterial material(&texture);

    const Colour emitted = material.Emitted(0.5, 0.5, Point3(0.0, 0.0, 0.0));

    REQUIRE(emitted.m_x == Approx(0.1));
    REQUIRE(emitted.m_y == Approx(0.2));
    REQUIRE(emitted.m_z == Approx(0.3));
}

TEST_CASE("DiffuseLightMaterial Scatter returns false", "[Material]")
{
    SolidColourTexture texture(Colour(1.0));
    const DiffuseLightMaterial material(&texture);

    const Ray ray(Point3(0.0, 0.0, 1.0), Vec3(0.0, 0.0, -1.0));
    const RayHitResult result = MakeFrontFacingResult(Point3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0));

    Colour attenuation;
    Ray out_ray;
    REQUIRE(material.Scatter(ray, result, attenuation, out_ray) == false);
}

} // namespace ART
