// Copyright Mia Rolfe. All rights reserved.
#include "Interval.h"

namespace ART
{

Interval::Interval(double min, double max)
{
    m_min = min;
    m_max = max;
}

Interval::Interval(const Interval& interval1, const Interval& interval2)
{
    m_min = interval1.m_min <= interval2.m_min ? interval1.m_min : interval2.m_min;
    m_max = interval1.m_max >= interval2.m_max ? interval1.m_max : interval2.m_max;

}

double Interval::Size() const
{
    return m_max - m_min;
}

bool Interval::Contains(double val) const
{
    return m_min <= val && val <= m_max;
}

bool Interval::Surrounds(double val) const
{
    return m_min < val && val < m_max;
}

double Interval::Clamp(double val) const
{
    if (val < m_min)
    {
        return m_min;
    }
    else if (val > m_max)
    {
        return m_max;
    }
    else
    {
        return val;
    }
}

Interval Interval::Expand(double delta) const
{
    const double padding = delta / 2.0;
	return Interval(m_min - padding, m_max + padding);
}

const Interval Interval::empty = Interval(infinity, -infinity);
const Interval Interval::universe = Interval(-infinity, infinity);

}; // namespace ART
