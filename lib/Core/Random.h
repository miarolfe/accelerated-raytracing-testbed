// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Core/Common.h>

namespace ART
{

// Returns a random number in [0, 1)
double RandomCanonicalDouble();

// Returns a random number in [min, max)
double RandomDouble(double min, double max);

} // namespace ART
