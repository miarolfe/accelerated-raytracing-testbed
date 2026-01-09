// Copyright Mia Rolfe. ALl rights reserved.
#pragma once

#include "Colour.h"
#include "Common.h"
#include "Image.h"
#include "Vec3.h"

namespace ART
{

struct Texture
{
public:
    virtual ~Texture() = default;

    virtual Colour Value(double u, double v, const Point3& p) const = 0;
};

struct SolidColourTexture : public Texture
{
public:
    SolidColourTexture(const Colour& albedo);

    SolidColourTexture(double red, double green, double blue);

    Colour Value(double u, double v, const Point3& point) const override;

protected:
    Colour m_albedo;
};

struct CheckerTexture : public Texture
{
public:
    CheckerTexture(double scale, Texture* even, Texture* odd);

    Colour Value(double u, double v, const Point3& point) const override;

protected:
    double m_inverse_scale;
    Texture* m_even_texture;
    Texture* m_odd_texture;
};

struct ImageTexture : public Texture
{
public:
    ImageTexture(const char* file_name) : m_image(file_name) {}

    Colour Value(double u, double v, const Point3& point) const override;

protected:
    Image m_image;
};

} // namespace ART
