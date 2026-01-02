// Copyright Mia Rolfe. All rights reserved.
#pragma once

namespace ART
{

struct AABB
{
public:
    double m_min_x = 0.0;
    double m_max_x = 0.0;
    double m_min_y = 0.0;
    double m_max_y = 0.0;
    double m_min_z = 0.0;
    double m_max_z = 0.0;

    // Default constructor
    AABB() = default;

    // Default destructor;
    ~AABB() = default;

    // Proper constructor
    AABB
    (
        double min_x, double max_x,
        double min_y, double max_y,
        double min_z, double max_z
    );

    // Copy constructor
    AABB(const AABB& other);

    // Copy assignment
    AABB& operator=(const AABB& other);

    // Move constructor
    AABB(AABB&& other) noexcept = delete;

    // Move assignment
    AABB& operator=(AABB&& other) noexcept = delete;
};

} // namespace ART
