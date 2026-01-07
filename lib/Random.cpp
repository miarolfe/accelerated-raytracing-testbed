// Copyright Mia Rolfe. All rights reserved.
#include "Random.h"

#include <random>

namespace ART
{

double RandomCanonicalDouble()
{
    // random_device ensures different RNG start
    static thread_local std::mt19937 generator(std::random_device{}());
    // Maps integer RNG to floating-point range [0, 1)
    static thread_local std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
}

double RandomDouble(double min, double max)
{
    return min + (max - min) * RandomCanonicalDouble();
}

} // namespace ART
