// Copyright Mia Rolfe. ALl rights reserved.
#include <Texture.h>

#include <Interval.h>

namespace ART
{

SolidColourTexture::SolidColourTexture(const Colour& albedo)
{
    m_albedo = albedo;
}

SolidColourTexture::SolidColourTexture(double red, double green, double blue)
{
    m_albedo = Colour(red, green, blue);
}

Colour SolidColourTexture::Value(double u, double v, const Point3& point) const
{
    return m_albedo;
}

CheckerTexture::CheckerTexture(double scale, Texture* even, Texture* odd)
{
    m_inverse_scale = 1.0 / scale;
    m_even_texture = even;
    m_odd_texture = odd;
}

Colour CheckerTexture::Value(double u, double v, const Point3& point) const
{
    const int32_t x_int = static_cast<int32_t>(std::floor(m_inverse_scale * point.m_x));
	const int32_t y_int = static_cast<int32_t>(std::floor(m_inverse_scale * point.m_y));
	const int32_t z_int = static_cast<int32_t>(std::floor(m_inverse_scale * point.m_z));

	const bool is_even = (((x_int + y_int + z_int) % 2) == 0);

	return is_even ? m_even_texture->Value(u, v, point) : m_odd_texture->Value(u, v, point);
}

Colour ImageTexture::Value(double u, double v, const Point3& point) const
{
    if (m_image.Width() <= 0)
	{
		return Colour(0.0, 1.0, 1.0);
	}

	u = Interval(0.0, 1.0).Clamp(u);
	v = 1.0 - Interval(0.0, 1.0).Clamp(v); // Flip V to image coordinates

	const int32_t i = static_cast<int32_t>(u * m_image.Width());
	const int32_t j = static_cast<int32_t>(v * m_image.Height());
	const uint8_t* pixel = m_image.PixelData(i, j);

	static constexpr double colour_scale = 1.0 / 255.0;
	return Colour(colour_scale * pixel[0], colour_scale * pixel[1], colour_scale * pixel[2]);
}

} // namespace ART
