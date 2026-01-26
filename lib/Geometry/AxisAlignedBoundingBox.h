// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Maths/Interval.h>
#include <Maths/Ray.h>
#include <Maths/Vec3.h>

namespace ART
{

// Collision struct
struct AABB
{
public:
    Interval m_x;
    Interval m_y;
    Interval m_z;

    // Default constructor
    AABB() = default;

    // Default destructor;
    ~AABB() = default;

    // Individual component value constructor
    AABB
    (
        double min_x, double max_x,
        double min_y, double max_y,
        double min_z, double max_z
    );

    // Min and max point constructor
    AABB(const Point3& min, const Point3& max);

    // Interval-based constructor
    AABB(const Interval& x, const Interval& y, const Interval& z);

    // Union constructor
    AABB(const AABB& boundingBox1, const AABB& boundingBox2);

    // Copy constructor
    AABB(const AABB& other);

    // Copy assignment
    AABB& operator=(const AABB& other);

    // Move constructor
    AABB(AABB&& other) noexcept = default;

    // Move assignment
    AABB& operator=(AABB&& other) noexcept = default;

    // Get component interval by value
    const Interval& operator[](std::size_t index) const;

    // Get component interval by reference
    Interval& operator[](std::size_t index);

    // Check if ray (bounded by interval) intersects with this AABB
    bool Hit(const Ray& ray, Interval rayT) const;

    // Return the longest axis of the AABB as an index where
    // x = 0, y = 1, z = 2
    std::size_t LongestAxis();

    // Calculate surface area of the AABB
    double SurfaceArea() const;

    void PadToMinimums();
};

} // namespace ART
