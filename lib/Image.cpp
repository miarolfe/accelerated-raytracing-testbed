// Copyright Mia Rolfe. All rights reserved.
#include "Image.h"
#include "Logger.h"
#include "../external/stb/stb_image.h"

namespace ART
{

Image::Image()
{
    m_image_width = 0;
    m_image_height = 0;
}

Image::Image(const char* file_name)
{
    const std::string image_file_name(file_name);

    if (Load("assets/" + image_file_name)) return;
    if (Load(image_file_name)) return;

    Logger::Get().LogError("Could not load image " + image_file_name);
}

Image::~Image()
{
    if (m_byte_data)
    {
        delete[] m_byte_data;
        delete[] m_fp_data;
    }
}

// Loads the linear (gamma=1) image data from the given file name. Returns true if the
// load succeeded. The resulting data buffer contains the three [0.0, 1.0]
// floating-point values for the first pixel (red, then green, then blue). Pixels are
// contiguous, going left to right for the width of the image, followed by the next row
// below, for the full height of the image.
bool Image::Load(const std::string& file_name)
{
    int temp_image_width;
    int temp_image_height;
    int temp;

    m_fp_data = stbi_loadf
    (
        file_name.c_str(),
        &temp_image_width,
        &temp_image_height,
        &temp,
        static_cast<int>(m_bytes_per_pixel)
    );
    if (!m_fp_data) return false;

    m_image_width = static_cast<std::size_t>(temp_image_width);
    m_image_height = static_cast<std::size_t>(temp_image_height);

    m_bytes_per_scanline = m_image_width * m_bytes_per_pixel;
    ConvertToBytes();
    return true;
}

static uint8_t magenta[] = { 255, 0, 255 };

const uint8_t* Image::PixelData(std::size_t x, std::size_t y) const
{
    if (!m_byte_data)
    {
        return magenta;
    }

    x = Clamp(x, 0, m_image_width);
    y = Clamp(y, 0, m_image_height);

    return m_byte_data + (y * m_bytes_per_scanline) + (x * m_bytes_per_scanline);
}

std::size_t Image::Width() const
{
    return m_image_width;
}

std::size_t Image::Height() const
{
    return m_image_height;
}

uint32_t Image::Clamp(std::size_t val, std::size_t low, std::size_t high)
{
    if (val < low)
    {
        return low;
    }
    else if (val < high)
    {
        return val;
    }
    else
    {
        return high - 1;
    }
}

uint8_t Image::FloatToByte(double val)
{
    if (val <= 0.0)
    {
        return 0.0;
    }
    else if (1.0 <= val)
    {
        return 255;
    }
    else
    {
        return static_cast<uint8_t>(256.0 * val);
    }
}

void Image::ConvertToBytes()
{
    const std::size_t total_bytes = m_image_width * m_image_height * Image::m_bytes_per_pixel;
    m_byte_data = new uint8_t[total_bytes];

    float* fp_data_ptr = m_fp_data;
    uint8_t* byte_data_ptr = m_byte_data;

    for (std::size_t i = 0; i < total_bytes; i++, fp_data_ptr++, byte_data_ptr++)
    {
        *byte_data_ptr = FloatToByte(*fp_data_ptr);
    }
}

} // namespace ART
