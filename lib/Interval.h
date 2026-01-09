// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Common.h>

namespace ART
{

struct Interval
{
public:
    double m_min = infinity;
    double m_max = -infinity;

    // Default constructor -> empty interval
    Interval() = default;

    Interval(double min, double max);

    // Union constructor
    Interval(const Interval& interval1, const Interval& interval2);

    // Return distance between max and min (NOT absolute, so negative results are possible)
    double Size() const;

    // Checks if val within interval inclusively
    bool Contains(double val) const;

    // Checks if val within interval exclusively
    bool Surrounds(double val) const;

    // Clamp val to the interval
    double Clamp(double val) const;

    // Pad out an interval to be more forgiving by (delta/2) on each side
    Interval Expand(double delta) const;

    static const Interval empty;
    static const Interval universe;
};

} // namespace ART
