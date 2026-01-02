// Copyright Mia Rolfe. All rights reserved.
#include "Random.h"

namespace ART
{
    double RandomCanonicalDouble()
    {
        return std::rand() / (RAND_MAX + 1.0);
    }

    double RandomDouble(double min, double max)
    {
        return min + (max - min) * RandomCanonicalDouble();
    }
} // namespace ART
