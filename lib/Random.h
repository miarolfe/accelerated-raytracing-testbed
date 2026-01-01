// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <cstdlib>

namespace ART
{
    // Returns a random number in [0, 1)
    inline double RandomCanonicalDouble()
    {
        return std::rand() / (RAND_MAX + 1.0);
    }

    // Returns a random number in [min, max)
    inline double RandomDouble(double min, double max)
    {
        return min + (max - min) * RandomCanonicalDouble();
    }
}
