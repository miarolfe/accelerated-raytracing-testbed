// Copyright Mia Rolfe. All rights reserved.
#include "AxisAlignedBoundingBox.h"

namespace ART
{
    AABB::AABB
    (
        double min_x, double max_x,
        double min_y, double max_y,
        double min_z, double max_z
    )
    {
        m_min_x = min_x;
        m_max_x = max_x;
        m_min_y = min_y;
        m_max_y = max_y;
        m_min_z = min_z;
        m_max_z = max_z;
    }

    AABB::AABB(const AABB& other) :
    m_min_x(other.m_min_x), m_max_x(other.m_max_x),
    m_min_y(other.m_min_y), m_max_y(other.m_max_y),
    m_min_z(other.m_min_z), m_max_z(other.m_max_z) {}

    AABB& AABB::operator=(const AABB& other)
    {
        m_min_x = other.m_min_x;
        m_max_x = other.m_max_x;
        m_min_y = other.m_min_y;
        m_max_y = other.m_max_y;
        m_min_z = other.m_min_z;
        m_max_z = other.m_max_z;

        return *this;
    }
} // namespace ART
