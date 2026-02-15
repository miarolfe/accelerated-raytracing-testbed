// Copyright Mia Rolfe. All rights reserved.
#include <Core/Random.h>

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

// Colour RNG stream for scene generation
static std::mt19937 s_colour_random_generator(std::random_device{}());
static std::uniform_real_distribution<double> s_colour_distribution(0.0, 1.0);

void SeedColourRNG(uint32_t seed)
{
    // No provided seed, pick a random one
    if (seed == 0)
    {
        s_colour_random_generator.seed(std::random_device{}());
    }
    else
    {
        s_colour_random_generator.seed(seed);
    }
    s_colour_distribution.reset();
}

double RandomColourDouble()
{
    return s_colour_distribution(s_colour_random_generator);
}

// Position RNG stream for scene generation
static std::mt19937 s_position_generator(std::random_device{}());
static std::uniform_real_distribution<double> s_position_distribution(0.0, 1.0);

void SeedPositionRNG(uint32_t seed)
{
    // No provided seed, pick a random one
    if (seed == 0)
    {
        s_position_generator.seed(std::random_device{}());
    }
    else
    {
        s_position_generator.seed(seed);
    }
    s_position_distribution.reset();
}

double RandomPositionDouble(double min, double max)
{
    return min + (max - min) * s_position_distribution(s_position_generator);
}

} // namespace ART
