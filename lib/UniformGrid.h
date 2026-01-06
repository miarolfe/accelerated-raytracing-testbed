// Copyright Mia Rolfe. All rights reserved.
#pragma once
#include "Common.h"
#include "IRayHittable.h"
#include "RayHitResult.h"

namespace ART
{

class UniformGrid : public IRayHittable
{
public:
    UniformGrid();

    UniformGrid(std::shared_ptr<IRayHittable> hittable);

    UniformGrid(std::vector<std::shared_ptr<IRayHittable>>& objects);

    void Clear();

    void Add(std::shared_ptr<IRayHittable> hittable);

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    std::vector<std::shared_ptr<IRayHittable>>& GetObjects();

    AABB BoundingBox() const override;
};

} // namespace ART
