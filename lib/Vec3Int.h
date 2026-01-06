// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include "Common.h"
#include "Random.h"

namespace ART
{

class Vec3Int
{
public:
    int m_x = 0;
    int m_y = 0;
    int m_z = 0;

    // Default constructor
    Vec3Int();

    // Construct with same value for all components
    Vec3Int(int val);

    // Generic constructor
    Vec3Int(int x, int y, int z);

    // Return Vec3Int with same component magntiudes but opposite signs
    Vec3Int operator-() const;

    // Get component by value
    int operator[](std::size_t index) const;

    // Get component by reference
    int& operator[](std::size_t index);

    // Add another Vec3Int to this Vec3Int
    Vec3Int& operator+=(const Vec3Int& other);

    // Multiply this Vec3Int by a scalar
    Vec3Int& operator*=(int t);

    // Divide this Vec3Int by a scalar
    Vec3Int& operator/=(int t);

    // Calculate the length of this Vec3Int
    double Length() const;

    // Calculate the length^2 of this Vec3Int
    double LengthSquared() const;

    std::string ToString() const;

    // Return a random Vec3Int with component values in the range [min, max)
    static Vec3Int Random(int min, int max);
};

// Add two Vec3Int together into one new Vec3Int
Vec3Int operator+(const Vec3Int& vec1, const Vec3Int& vec2);

// Subtract one Vec3Int from another into one new Vec3Int
Vec3Int operator-(const Vec3Int& vec1, const Vec3Int& vec2);

// Multiply two Vec3Int by each other into one new Vec3Int
Vec3Int operator*(const Vec3Int& vec1, const Vec3Int& vec2);

// Scale one Vec3Int by a scalar into a new Vec3Int
Vec3Int operator*(const Vec3Int& vec, int t);

// Scale one Vec3Int by a scalar into a new Vec3Int
Vec3Int operator*(int t, const Vec3Int& vec);

// Divide one Vec3Int by a scalar into a new Vec3Int
Vec3Int operator/(const Vec3Int& vec, int t);

// Alias for Vec3Int for points
using Point3Int = Vec3Int;

} // namespace ART
