// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include "Common.h"

namespace ART
{

struct Image
{
public:
    Image();

    Image(const char* file_name);

    ~Image();

    bool Load(const std::string& file_name);

    const uint8_t* PixelData(std::size_t x, std::size_t y) const;

    std::size_t Width() const;

    std::size_t Height() const;

protected:
    static uint32_t Clamp(std::size_t val, std::size_t low, std::size_t high);

    static uint8_t FloatToByte(double val);

    void ConvertToBytes();

    float* m_fp_data = nullptr;
    uint8_t* m_byte_data = nullptr;
    std::size_t m_image_width;
    std::size_t m_image_height;
    std::size_t m_bytes_per_scanline;
    static const int8_t m_bytes_per_pixel = 3;
};

}
