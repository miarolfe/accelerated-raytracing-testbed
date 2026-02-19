// Copyright Mia Rolfe. All rights reserved.
#include <Maths/Vec3Int.h>

#include <Core/Random.h>

namespace ART
{

Vec3Int::Vec3Int() {}

Vec3Int::Vec3Int(int val)
{
    m_x = val;
    m_y = val;
    m_z = val;
}

Vec3Int::Vec3Int(int x, int y, int z)
{
    m_x = x;
    m_y = y;
    m_z = z;
}

Vec3Int Vec3Int::operator-() const
{
    return Vec3Int(-m_x, -m_y, -m_z);
}

int Vec3Int::operator[](std::size_t index) const
{
    assert(index < 3);
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
    return 0;
}

int& Vec3Int::operator[](std::size_t index)
{
    assert(index < 3);
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

Vec3Int& Vec3Int::operator+=(const Vec3Int& other)
{
    m_x += other.m_x;
    m_y += other.m_y;
    m_z += other.m_z;
    return *this;
}

Vec3Int& Vec3Int::operator*=(int t)
{
    m_x *= t;
    m_y *= t;
    m_z *= t;
    return *this;
}

Vec3Int& Vec3Int::operator/=(int t)
{
    m_x /= t;
    m_y /= t;
    m_z /= t;
    return *this;
}

double Vec3Int::Length() const
{
    return std::sqrt(LengthSquared());
}

double Vec3Int::LengthSquared() const
{
    return (m_x * m_x) + (m_y * m_y) + (m_z * m_z);
}

std::string Vec3Int::ToString() const
{
    return std::string("(" + std::to_string(m_x) + ", " + std::to_string(m_y) + ", " + std::to_string(m_z) + ")");
}

Vec3Int Vec3Int::Random(int min, int max)
{
    return static_cast<int32_t>(RandomDouble(min, max + 1.0));
}

Vec3Int operator+(const Vec3Int& vec1, const Vec3Int& vec2)
{
    return Vec3Int(vec1.m_x + vec2.m_x, vec1.m_y + vec2.m_y, vec1.m_z + vec2.m_z);
}

Vec3Int operator-(const Vec3Int& vec1, const Vec3Int& vec2)
{
    return Vec3Int(vec1.m_x - vec2.m_x, vec1.m_y - vec2.m_y, vec1.m_z - vec2.m_z);
}

Vec3Int operator*(const Vec3Int& vec1, const Vec3Int& vec2)
{
    return Vec3Int(vec1.m_x * vec2.m_x, vec1.m_y * vec2.m_y, vec1.m_z * vec2.m_z);
}

Vec3Int operator*(const Vec3Int& vec, int t)
{
    return Vec3Int(vec.m_x * t, vec.m_y * t, vec.m_z * t);
}

Vec3Int operator*(int t, const Vec3Int& vec)
{
    return vec * t;
}

Vec3Int operator/(const Vec3Int& vec, int t)
{
    return Vec3Int(vec.m_x / t, vec.m_y / t, vec.m_z / t);
}

} // namespace ART
