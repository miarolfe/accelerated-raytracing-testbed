// Copyright Mia Rolfe. All rights reserved.
#pragma once
#include <cassert>
#include <cmath>
#include <cstdint>

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
    Vec3()
    {
        m_x = 0.0;
        m_y = 0.0;
        m_z = 0.0;
    }

    // Construct with same value for all components
    Vec3(double val)
    {
        m_x = val;
        m_y = val;
        m_z = val;
    }

    // Generic constructor
    Vec3(double x, double y, double z)
    {
        m_x = x;
        m_y = y;
        m_z = z;
    }

    // Return inverse of this Vec3
    Vec3 operator-() const
    {
        return Vec3(-m_x, -m_y, -m_z);
    }

    // Get component by value
    double operator[](std::size_t index) const
    {
        assert(index >= 0 && index <= 2);
        switch (index)
        {
        case 0:
            return m_x;
        case 1:
            return m_y;
        case 2:
            return m_z;
        }

        // Should never be reachable in valid code
        assert(false);
        return 0.0;
    }

    // Get component by reference
    double& operator[](std::size_t index)
    {
        assert(index >= 0 && index <= 2);
        switch (index)
        {
        case 0:
            return m_x;
        case 1:
            return m_y;
        case 2:
            return m_z;
        }

        // Should never be reachable in valid code
        assert(false);
        return m_x;
    }

    // Add another Vec3 to this Vec3
    Vec3& operator+=(const Vec3& other)
    {
        m_x += other.m_x;
        m_y += other.m_y;
        m_z += other.m_z;
        return *this;
    }

    // Multiply this Vec3 by a scalar
    Vec3& operator*=(double t)
    {
        m_x *= t;
        m_y *= t;
        m_z *= t;
        return *this;
    }

    // Divide this Vec3 by a scalar
    Vec3& operator/=(double t)
    {
        return (*this *= (1.0 / t));
    }

    // Calculate the magnitude of this Vec3
    double Length() const
    {
        return std::sqrt(LengthSquared());
    }

    // Calculate the length^2 of this Vec3
    double LengthSquared() const
    {
        return (m_x * m_x) + (m_y * m_y) + (m_z * m_z);
    }

    // Return if components close enough to zero to effectively be all zero (FP issue)
    bool NearZero() const
    {
        static constexpr double close_to_zero_value = 1e-8;
        bool x_near_zero = (std::fabs(m_x) < close_to_zero_value);
        bool y_near_zero = (std::fabs(m_y) < close_to_zero_value);
        bool z_near_zero = (std::fabs(m_z) < close_to_zero_value);

        return x_near_zero && y_near_zero && z_near_zero;
    }

    // Return a random Vec3 with component values in the range [0, 1)
    static Vec3 Random()
    {
        return Vec3
        (
            RandomCanonicalDouble(),
            RandomCanonicalDouble(),
            RandomCanonicalDouble()
        );
    }

    // Return a random Vec3 with component values in the range [min, max)
    static Vec3 Random(double min, double max)
    {
        return Vec3
        (
            RandomDouble(min, max),
            RandomDouble(min, max),
            RandomDouble(min, max)
        );
    }
};

inline Vec3 operator+(const Vec3& vec1, const Vec3& vec2)
{
    return Vec3(vec1.m_x + vec2.m_x, vec1.m_y + vec2.m_y, vec1.m_z + vec2.m_z);
}

inline Vec3 operator-(const Vec3& vec1, const Vec3& vec2)
{
    return Vec3(vec1.m_x - vec2.m_x, vec1.m_y - vec2.m_y, vec1.m_z - vec2.m_z);
}

inline Vec3 operator*(const Vec3& vec1, const Vec3& vec2)
{
    return Vec3(vec1.m_x * vec2.m_x, vec1.m_y * vec2.m_y, vec1.m_z * vec2.m_z);
}

inline Vec3 operator*(const Vec3& vec, double t)
{
    return Vec3(vec.m_x * t, vec.m_y * t, vec.m_z * t);
}

inline Vec3 operator*(double t, const Vec3& vec)
{
    return vec * t;
}

inline Vec3 operator/(const Vec3& vec, double t)
{
    return (1.0 / t) * vec;
}

inline double Dot(const Vec3& vec1, const Vec3& vec2)
{
    return (vec1.m_x * vec2.m_x) + (vec1.m_y * vec2.m_y) + (vec1.m_z * vec2.m_z);
}

inline Vec3 Cross(const Vec3& vec1, const Vec3& vec2)
{
    return Vec3
    (
        vec1.m_y * vec2.m_z - vec1.m_z * vec2.m_y,
        vec1.m_z * vec2.m_x - vec1.m_x * vec2.m_z,
        vec1.m_x * vec2.m_y - vec1.m_y * vec2.m_x
    );
}

inline Vec3 Normalised(const Vec3& vec)
{
    return vec / vec.Length();
}

inline Vec3 RandomInUnitDisk()
{
    while (true)
    {
        const Vec3 point = Vec3(RandomDouble(-1.0, 1.0), RandomDouble(-1.0, 1.0), 0.0);
        if (point.LengthSquared() < 1.0)
        {
            return point;
        }
    }

    // Should never be reachable in valid code
    assert(false);
    return Vec3();
}

inline Vec3 RandomNormalised()
{
    while (true)
    {
        const Vec3 random_vec = Vec3::Random(-1.0, 1.0);
        const double random_vec_length_squared = random_vec.LengthSquared();
        static constexpr double min_safe_fp_magntiude = 1e-160;
        if (min_safe_fp_magntiude < random_vec_length_squared && random_vec_length_squared <= 1)
        {
            return random_vec / std::sqrt(random_vec_length_squared);
        }
    }
}

inline Vec3 RandomOnHemisphere(const Vec3& normal)
{
    const Vec3 vec_on_unit_sphere = RandomNormalised();
    if (Dot(vec_on_unit_sphere, normal) > 0.0)
    {
        return vec_on_unit_sphere;
    }
    else
    {
        return -vec_on_unit_sphere;
    }
}

inline Vec3 Reflect(const Vec3& v, const Vec3& n)
{
    return v - 2.0 * Dot(v, n) * n;
}

inline Vec3 Refract(const Vec3& uv, const Vec3& n, double e_tai_over_e_tat)
{
    const double cos_theta = std::fmin(Dot(-uv, n), 1.0);
    const Vec3 r_out_perpendicular = e_tai_over_e_tat * (uv + (cos_theta * n));
    const Vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perpendicular.LengthSquared())) * n;
    return r_out_perpendicular + r_out_parallel;
}

using Point3 = Vec3;

}
