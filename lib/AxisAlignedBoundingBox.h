// Copyright Mia Rolfe. All rights reserved.
#pragma once

namespace ART
{

struct AABB
{
public:
    AABB
    (
        double _min_x,
        double _max_x,
        double _min_y,
        double _max_y,
        double _min_z,
        double _max_z
    )
    {
        min_x = _min_x;
        max_x = _max_x;
        min_y = _min_y;
        max_y = _max_y;
        min_z = _max_z;
    }

    AABB(const AABB& other) : min_x(other.min_x), max_x(other.max_x), min_y(other.min_y), max_y(other.max_y), min_z(other.min_z), max_z(other.max_z) {}

    double min_x = 0.0;
    double max_x = 0.0;
    double min_y = 0.0;
    double max_y = 0.0;
    double min_z = 0.0;
    double max_z = 0.0;
};

};
