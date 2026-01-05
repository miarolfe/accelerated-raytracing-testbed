// Copyright Mia Rolfe. All rights reserved.
#include "Material.h"
#include "Random.h"
#include "RayHitResult.h"
#include "Texture.h"
#include "Vec3.h"
#include <algorithm>
#include <memory>

namespace ART
{

Colour Material::Emitted(double u, double v, const Point3& point) const
{
    return Colour(0.0);
}

bool Material::Scatter(const Ray& ray, const RayHitResult& result, Colour& out_attenuation, Ray& out_ray) const
{
    return false;
}

LambertianMaterial::LambertianMaterial(const Colour& albedo)
{
    m_texture = std::make_shared<SolidColourTexture>(albedo);
}

LambertianMaterial::LambertianMaterial(std::shared_ptr<Texture> texture)
{
    m_texture = texture;
}

bool LambertianMaterial::Scatter(const Ray& ray, const RayHitResult& result, Colour& out_attenuation, Ray& out_ray) const
{
    Vec3 scatter_direction = RandomOnHemisphere(Normalised(result.m_normal));
    if (scatter_direction.NearZero())
    {
        scatter_direction = result.m_normal;
    }

    out_ray = Ray(result.m_point, Normalised(scatter_direction), ray.m_time);
    out_attenuation = m_texture->Value(result.m_u, result.m_v, result.m_point);
    return true;
}

MetalMaterial::MetalMaterial(const Colour& albedo, double fuzz)
{
    m_albedo = albedo;
	m_fuzz = std::clamp(fuzz, 0.0, 1.0);
}

bool MetalMaterial::Scatter(const Ray& ray, const RayHitResult& result, Colour& out_attenuation, Ray& out_ray) const
{
    const Vec3 reflected_direction = Normalised(Reflect(Normalised(ray.m_direction), Normalised(result.m_normal)));
	const Vec3 fuzzed_direction = Normalised(reflected_direction + (m_fuzz * RandomNormalised()));
	out_ray = Ray(result.m_point, fuzzed_direction, ray.m_time);
	out_attenuation = m_albedo;
	return (Dot(out_ray.m_direction, Normalised(result.m_normal)) > 0);
}

DielectricMaterial::DielectricMaterial(double refraction_index)
{
    m_refraction_index = refraction_index;
}

bool DielectricMaterial::Scatter(const Ray& ray, const RayHitResult& result, Colour& out_attenuation, Ray& out_ray) const
{
    out_attenuation = Colour(1.0);

    const Vec3 normalised_normal = Normalised(result.m_normal);
    const Vec3 normalised_direction = Normalised(ray.m_direction);

    const double refraction_ratio = result.m_is_front_facing ? (1.0 / m_refraction_index) : m_refraction_index;

    const double cos_theta = std::clamp(Dot(-normalised_direction, normalised_normal), 0.0, 1.0);
    const double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

    const bool should_reflect = (refraction_ratio * sin_theta > 1.0) || (Reflectance(cos_theta, refraction_ratio) > RandomCanonicalDouble());

    const Vec3 scatter_direction = should_reflect ?
        Reflect(normalised_direction, normalised_normal) :
        Refract(normalised_direction, normalised_normal, refraction_ratio);

    out_ray = Ray(result.m_point, scatter_direction, ray.m_time);

    return true;
}

double DielectricMaterial::Reflectance(double cosine, double refraction_index)
{
    double r0 = (1.0 - refraction_index) / (1.0 + refraction_index);
	r0 = r0 * r0;
	return r0 + (1.0 - r0) * std::pow((1.0 - cosine), 5);
}

DiffuseLightMaterial::DiffuseLightMaterial(std::shared_ptr<Texture> texture)
{
    m_texture = texture;
}

DiffuseLightMaterial::DiffuseLightMaterial(const Colour& emit_colour)
{
    m_texture = std::make_shared<SolidColourTexture>(emit_colour);
}

Colour DiffuseLightMaterial::Emitted(double u, double v, const Point3& point) const
{
    return m_texture->Value(u, v, point);
}

} // namespace ART
