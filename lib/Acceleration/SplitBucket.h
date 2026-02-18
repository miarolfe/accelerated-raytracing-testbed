// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Geometry/AxisAlignedBoundingBox.h>

namespace ART
{

struct SplitBucket
{
    AABB bounding_box;
    std::size_t num_hittables = 0;
};

} // namespace ART
