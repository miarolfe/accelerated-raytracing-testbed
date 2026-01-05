// Copyright Mia Rolfe. All rights reserved.
#include "Colour.h"
#include <cmath>

namespace ART
{

double LinearToGamma(double linear_colour)
{
    if (linear_colour > 0.0)
    {
        return std::sqrt(linear_colour);
    }
    else
    {
        return 0.0;
    }
}

} // namespace ART
