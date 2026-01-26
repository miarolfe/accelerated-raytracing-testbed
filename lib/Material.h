// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Colour.h>
#include <Common.h>
#include <RayHitResult.h>
#include <Texture.h>

namespace ART
{

class Material
{
public:
    virtual ~Material() = default;

    virtual Colour Emitted(double u, double v, const Point3& point) const;

    virtual bool Scatter(const Ray& ray, const RayHitResult& result, Colour& out_attenuation, Ray& out_ray) const;
};

class LambertianMaterial : public Material
{
public:
    LambertianMaterial(Texture* texture);

    bool Scatter(const Ray& ray, const RayHitResult& result, Colour& out_attenuation, Ray& out_ray) const override;

protected:
    Texture* m_texture;
};

class MetalMaterial : public Material
{
public:
    MetalMaterial(const Colour& albedo, double fuzz);

    bool Scatter(const Ray& ray, const RayHitResult& result, Colour& out_attenuation, Ray& out_ray) const override;

protected:
    Colour m_albedo;
    double m_fuzz;
};

class DielectricMaterial : public Material
{
public:
    DielectricMaterial(double refraction_index);

    bool Scatter(const Ray& ray, const RayHitResult& result, Colour& out_attenuation, Ray& out_ray) const override;

protected:
    // Uses Schlick's approximation for reflectance
    static double Reflectance(double cosine, double refraction_index);

    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
	// the refractive index of the enclosing media
    double m_refraction_index;
};

class DiffuseLightMaterial : public Material
{
public:
    DiffuseLightMaterial(Texture* texture);

    Colour Emitted(double u, double v, const Point3& point) const override;

protected:
    Texture* m_texture;
};

} // namespace ART
