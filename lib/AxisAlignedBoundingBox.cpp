// Copyright Mia Rolfe. All rights reserved.
#include <AxisAlignedBoundingBox.h>

namespace ART
{

AABB::AABB
(
    double min_x, double max_x,
    double min_y, double max_y,
    double min_z, double max_z
)
{
    m_x = Interval(min_x, max_x);
    m_y = Interval(min_y, max_y);
    m_z = Interval(min_z, max_z);
}

AABB::AABB(const Point3& min, const Point3& max)
{
    m_x = (min[0] <= max[0]) ? Interval(min[0], max[0]) : Interval(max[0], min[0]);
	m_y = (min[1] <= max[1]) ? Interval(min[1], max[1]) : Interval(max[1], min[1]);
	m_z = (min[2] <= max[2]) ? Interval(min[2], max[2]) : Interval(max[2], min[2]);

    PadToMinimums();
}

AABB::AABB(const Interval& x, const Interval& y, const Interval& z)
{
    m_x = x;
	m_y = y;
	m_z = z;

	PadToMinimums();
}

AABB::AABB(const AABB& boundingBox1, const AABB& boundingBox2)
{
    m_x = Interval(boundingBox1.m_x, boundingBox2.m_x);
    m_y = Interval(boundingBox1.m_y, boundingBox2.m_y);
    m_z = Interval(boundingBox1.m_z, boundingBox2.m_z);
}

AABB::AABB(const AABB& other) :
m_x(other.m_x), m_y(other.m_y), m_z(other.m_z) {}

AABB& AABB::operator=(const AABB& other)
{
    m_x = other.m_x;
    m_y = other.m_y;
    m_z = other.m_z;

    return *this;
}

const Interval& AABB::operator[](std::size_t index) const
{
    // Only 3 axes in 3D space...
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

Interval& AABB::operator[](std::size_t index)
{
    // Only 3 axes in 3D space...
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

bool AABB::Hit(const Ray& ray, Interval rayT) const
{
    const Point3& rayOrigin = ray.m_origin;
	const Vec3& rayDirection = ray.m_direction;

	for (std::size_t axis = 0; axis < 3; axis++)
	{
		const Interval& axisInterval = (*this)[axis];
		const double rayDirectionAxisInverse = 1.0 / rayDirection[axis];

		const double t0 = (axisInterval.m_min - rayOrigin[axis]) * rayDirectionAxisInverse;
		const double t1 = (axisInterval.m_max - rayOrigin[axis]) * rayDirectionAxisInverse;

		if (t0 < t1)
		{
			if (t0 > rayT.m_min)
			{
				rayT.m_min = t0;
			}
			if (t1 < rayT.m_max)
			{
				rayT.m_max = t1;
			}
		}
		else
		{
			if (t1 > rayT.m_min)
			{
				rayT.m_min = t1;
			}
			if (t0 < rayT.m_max)
			{
				rayT.m_max = t0;
			}
		}

		if (rayT.m_min > rayT.m_max)
		{
			return false;
		}
	}

	return true;
}

std::size_t AABB::LongestAxis()
{
    if (m_x.Size() > m_y.Size())
    {
   	    return m_x.Size() > m_z.Size() ? 0 : 2;
    }
    else
    {
       	return m_y.Size() > m_z.Size() ? 1 : 2;
    }
}

void AABB::PadToMinimums()
{
    static constexpr double delta = 0.0001;

    if (m_x.Size() < delta)
	{
		m_x = m_x.Expand(delta);
	}

	if (m_y.Size() < delta)
	{
		m_y = m_y.Expand(delta);
	}

	if (m_z.Size() < delta)
	{
		m_z = m_z.Expand(delta);
	}
}

} // namespace ART
