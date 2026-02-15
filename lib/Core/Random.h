// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Core/Common.h>

namespace ART
{

// Returns a random number in [0, 1)
double RandomCanonicalDouble();

// Returns a random number in [min, max)
double RandomDouble(double min, double max);

// Scene generation RNG streams
// Colour and position can be generated deterministically

// Seed value 0 = non-deterministic
void SeedColourRNG(uint32_t seed);
void SeedPositionRNG(uint32_t seed);

// Returns a random number in [0, 1) from the colour stream
double RandomColourDouble();

// Returns a random number in [min, max) from the position stream
double RandomPositionDouble(double min, double max);

} // namespace ART
