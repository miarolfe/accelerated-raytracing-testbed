// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include "Common.h"
#include "Random.h"

namespace ART
{

class Vec3
{
public:
    double m_x = 0.0;
    double m_y = 0.0;
    double m_z = 0.0;

    // Default constructor
    Vec3();

    // Construct with same value for all components
    Vec3(double val);

    // Generic constructor
    Vec3(double x, double y, double z);

    // Return Vec3 with same component magntiudes but opposite signs
    Vec3 operator-() const;

    // Get component by value
    double operator[](std::size_t index) const;

    // Get component by reference
    double& operator[](std::size_t index);

    // Add another Vec3 to this Vec3
    Vec3& operator+=(const Vec3& other);

    // Multiply this Vec3 by a scalar
    Vec3& operator*=(double t);

    // Divide this Vec3 by a scalar
    Vec3& operator/=(double t);

    // Calculate the magnitude of this Vec3
    double Length() const;

    // Calculate the length^2 of this Vec3
    double LengthSquared() const;

    // Return if components close enough to zero to effectively be all zero (FP issue)
    bool NearZero() const;

    std::string ToString() const;

    // Return a random Vec3 with component values in the range [0, 1)
    static Vec3 Random();

    // Return a random Vec3 with component values in the range [min, max)
    static Vec3 Random(double min, double max);
};

// Add two Vec3 together into one new Vec3
Vec3 operator+(const Vec3& vec1, const Vec3& vec2);

// Subtract one Vec3 from another into one new Vec3
Vec3 operator-(const Vec3& vec1, const Vec3& vec2);

// Multiply two Vec3 by each other into one new Vec3
Vec3 operator*(const Vec3& vec1, const Vec3& vec2);

// Scale one Vec3 by a scalar into a new Vec3
Vec3 operator*(const Vec3& vec, double t);

// Scale one Vec3 by a scalar into a new Vec3
Vec3 operator*(double t, const Vec3& vec);

// Divide one Vec3 by a scalar into a new Vec3
Vec3 operator/(const Vec3& vec, double t);

// Calculate the dot product of two Vec3 into a new Vec3
double Dot(const Vec3& vec1, const Vec3& vec2);

// Calculate the cross product of two Vec3 into a new Vec3
Vec3 Cross(const Vec3& vec1, const Vec3& vec2);

// Calculate the normalised form of a Vec3 into a new Vec3
Vec3 Normalised(const Vec3& vec);

// Calculate a random Vec3 on the a X-Y unit disk
Vec3 RandomInUnitDisk();

// Calculate a random normalised Vec3
Vec3 RandomNormalised();

// Calculate a random normalised Vec3 on a unit hemisphere
Vec3 RandomOnHemisphere(const Vec3& normal);

// Reflect a Vec3
Vec3 Reflect(const Vec3& v, const Vec3& n);

// Refract a Vec3
Vec3 Refract(const Vec3& uv, const Vec3& n, double e_tai_over_e_tat);

// Alias for Vec3 for points
using Point3 = Vec3;

} // namespace ART
