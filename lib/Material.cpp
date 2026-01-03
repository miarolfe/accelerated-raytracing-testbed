// Copyright Mia Rolfe. All rights reserved.
#include "Material.h"
#include "RayHitResult.h"
#include "Texture.h"
#include "Vec3.h"
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
    Vec3 scatter_direction = result.m_normal + RandomNormalised();
    if (scatter_direction.NearZero())
    {
        scatter_direction = result.m_normal;
    }

    out_ray = Ray(result.m_point, scatter_direction, ray.m_time);
    out_attenuation = m_texture->Value(result.m_u, result.m_v, result.m_point);
    return true;
}

MetalMaterial::MetalMaterial(const Colour& albedo, double fuzz)
{
    m_albedo = albedo;
	m_fuzz = fuzz;
}

bool MetalMaterial::Scatter(const Ray& ray, const RayHitResult& result, Colour& out_attenuation, Ray& out_ray) const
{
    Vec3 reflected_direction = Reflect(ray.m_direction, result.m_normal);
	reflected_direction = Normalised(reflected_direction) + (m_fuzz * RandomNormalised());
	out_ray = Ray(result.m_point, reflected_direction, ray.m_time);
	out_attenuation = m_albedo;
	return (Dot(out_ray.m_direction, result.m_normal) > 0);
}

DielectricMaterial::DielectricMaterial(double refraction_index)
{
    m_refraction_index = refraction_index;
}

bool DielectricMaterial::Scatter(const Ray& ray, const RayHitResult& result, Colour& out_attenuation, Ray& out_ray) const
{
    out_attenuation = Colour(1.0, 1.0, 1.0);
	const double actual_refractive_index = result.m_is_front_facing ? (1.0 / m_refraction_index) : m_refraction_index;

	const Vec3 normalised_direction = Normalised(ray.m_direction);
	const double cos_theta = std::fmin(Dot(-normalised_direction, result.m_normal), 1.0);
	const double sinTheta = std::sqrt(1.0 - (cos_theta * cos_theta));

	const bool cannot_refract = (actual_refractive_index * sinTheta) > 1.0;
	Vec3 direction;

	if (cannot_refract || (Reflectance(cos_theta, actual_refractive_index) > RandomCanonicalDouble()))
	{
		direction = Reflect(normalised_direction, result.m_normal);
	}
	else
	{
		direction = Refract(normalised_direction, result.m_normal, actual_refractive_index);
	}

	out_ray = Ray(result.m_point, direction, ray.m_time);
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
