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
    // Unrolled branchless loop :)

    // For each axis, compute t values where ray crosses the min/max planes
    const double t0_x = (m_x.m_min - ray.m_origin.m_x) * ray.m_inverse_direction.m_x;
    const double t1_x = (m_x.m_max - ray.m_origin.m_x) * ray.m_inverse_direction.m_x;
    const double t0_y = (m_y.m_min - ray.m_origin.m_y) * ray.m_inverse_direction.m_y;
    const double t1_y = (m_y.m_max - ray.m_origin.m_y) * ray.m_inverse_direction.m_y;
    const double t0_z = (m_z.m_min - ray.m_origin.m_z) * ray.m_inverse_direction.m_z;
    const double t1_z = (m_z.m_max - ray.m_origin.m_z) * ray.m_inverse_direction.m_z;

    // Find entry and exit per-axis
    const double t_near_x = t0_x < t1_x ? t0_x : t1_x;
    const double t_far_x  = t0_x > t1_x ? t0_x : t1_x;
    const double t_near_y = t0_y < t1_y ? t0_y : t1_y;
    const double t_far_y  = t0_y > t1_y ? t0_y : t1_y;
    const double t_near_z = t0_z < t1_z ? t0_z : t1_z;
    const double t_far_z  = t0_z > t1_z ? t0_z : t1_z;

    // t_min = max of all near values (latest entry)
    double t_min = rayT.m_min;

    // t_max = min of all far values (earliest exit)
    double t_max = rayT.m_max;

    t_min = t_near_x > t_min ? t_near_x : t_min;
    t_min = t_near_y > t_min ? t_near_y : t_min;
    t_min = t_near_z > t_min ? t_near_z : t_min;

    t_max = t_far_x < t_max ? t_far_x : t_max;
    t_max = t_far_y < t_max ? t_far_y : t_max;
    t_max = t_far_z < t_max ? t_far_z : t_max;

    // If ray enters before it exits, have intersected
    return t_min <= t_max;
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

double AABB::SurfaceArea() const
{
    const double dx = m_x.Size();
    const double dy = m_y.Size();
    const double dz = m_z.Size();
    return 2.0 * (dx * dy + dy * dz + dz * dx);
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
